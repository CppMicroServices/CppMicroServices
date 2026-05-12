/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#include "BundlePrivate.h"
#include "BundleStorage.h"

#include "states/BundleActiveState.hpp"
#include "states/BundleInstalledState.hpp"
#include "states/BundleLifecycleState.hpp"
#include "states/BundleResolvedState.hpp"
#include "states/BundleStartingState.hpp"
#include "states/BundleStoppingState.hpp"
#include "states/BundleUninstalledState.hpp"

#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/BundleResource.h"
#include "cppmicroservices/BundleResourceStream.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/ServiceRegistration.h"
#include "cppmicroservices/SharedLibraryException.h"

#include "cppmicroservices/util/Error.h"
#include "cppmicroservices/util/FileSystem.h"
#include "cppmicroservices/util/String.h"

#include "BundleArchive.h"
#include "BundleContextPrivate.h"
#include "BundleResourceContainer.h"
#include "BundleUtils.h"
#include "CoreBundleContext.h"
#include "ServiceReferenceBasePrivate.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstring>
#include <iterator>

namespace cppmicroservices
{

    Bundle
    MakeBundle(std::shared_ptr<BundlePrivate> const& d)
    {
        return Bundle(d);
    }

    void
    BundlePrivate::Stop(uint32_t options)
    {
        {
            auto l = this->Lock();
            US_UNUSED(l);

            if (GetBundleStateEnum() == Bundle::STATE_UNINSTALLED)
            {
                throw std::logic_error("Bundle " + symbolicName + " (location=" + location + ") is uninstalled");
            }

            if ((options & Bundle::STOP_TRANSIENT) == 0)
            {
                SetAutostartSetting(-1);
            }
        }

        auto savedException = GetLifecycleState()->Stop(*this, options);

        if (savedException)
        {
            std::rethrow_exception(savedException);
        }
    }

    std::exception_ptr
    BundlePrivate::DeactivateBundle()
    {
        std::exception_ptr res;

        coreCtx->listeners.BundleChanged(
            BundleEvent(BundleEvent::BUNDLE_STOPPING, MakeBundle(this->shared_from_this())));

        if (wasStarted && bactivator != nullptr)
        {
            try
            {
                bactivator->Stop(MakeBundleContext(bundleContext.Load()));
            }
            catch (...)
            {
                res = std::make_exception_ptr(
                    std::runtime_error("Bundle " + symbolicName + " (location=" + location
                                       + "), BundleActivator::Stop() failed: " + util::GetLastExceptionStr()));
            }

            // if stop was aborted (uninstall or timeout), check aborted/state
            {
                std::string cause;
                if (aborted == static_cast<uint8_t>(Aborted::YES))
                {
                    if (Bundle::STATE_UNINSTALLED == GetBundleStateEnum())
                    {
                        cause = "Bundle uninstalled during Stop()";
                    }
                    else
                    {
                        cause = "Bundle activator Stop() time-out";
                    }
                }
                else
                {
                    aborted = static_cast<uint8_t>(Aborted::NO);
                    if (Bundle::STATE_STOPPING != GetBundleStateEnum())
                    {
                        cause = "Bundle changed state because of refresh during Stop()";
                    }
                }
                if (!cause.empty())
                {
                    res = std::make_exception_ptr(std::runtime_error("Bundle " + symbolicName + " (location=" + location
                                                                     + ") stop failed: " + cause));
                }
            }
            bactivator = nullptr;
        }

        InvalidateBundleContext();

        return res;
    }

    void
    BundlePrivate::InvalidateBundleContext()
    {
        // Call hooks after we've called BundleActivator::Stop(), but before we've
        // cleared all resources
        std::shared_ptr<BundleContextPrivate> ctx = bundleContext.Load();
        if (ctx)
        {
            coreCtx->listeners.HooksBundleStopped(ctx);
            RemoveBundleResources();
            ctx->Invalidate();
            bundleContext.Store(std::shared_ptr<BundleContextPrivate>());
        }
    }

    Bundle::State
    BundlePrivate::GetUpdatedState()
    {
        return GetLifecycleState()->GetUpdatedState(*this);
    }

    void
    BundlePrivate::SetStateInstalled(bool sendEvent)
    {
        // Make sure that bundleContext is invalid
        std::shared_ptr<BundleContextPrivate> ctx;
        if ((ctx = bundleContext.Exchange(ctx)))
        {
            ctx->Invalidate();
        }

        SetLifecycleState(std::make_shared<BundleInstalledState>());
        if (sendEvent)
        {
            coreCtx->listeners.BundleChanged({ BundleEvent::BUNDLE_RESOLVED, MakeBundle(this->shared_from_this()) });
        }
    }

    void
    BundlePrivate::ResolveAndActivate()
    {
        // Auto-resolve if needed (INSTALLED -> RESOLVED), then delegate to current state
        GetUpdatedState();
        GetLifecycleState()->Start(*this, 0);
    }

    void
    BundlePrivate::Uninstall()
    {
        // Delegate to current state object
        GetLifecycleState()->Uninstall(*this);

        coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::BUNDLE_UNINSTALLED, MakeBundle(shared_from_this())));
    }

    std::string
    BundlePrivate::GetLocation() const
    {
        return location;
    }

    void
    BundlePrivate::Start(uint32_t options)
    {
        // Hold the framework shutdown blocker through activation to prevent
        // the framework from stopping while this bundle is starting.
        std::unique_ptr<FrameworkShutdownBlocker> frameworkBlock = nullptr;
        {
            auto l = this->Lock();
            US_UNUSED(l);
            frameworkBlock = coreCtx->GetFrameworkStateAndBlock();
            if (frameworkBlock->frameworkHasStopped)
            {
                throw std::runtime_error("Bundle " + symbolicName + " (location=" + location
                                         + ") belongs to a stopped framework");
            }

            auto state = GetBundleStateEnum();
            if (state == Bundle::STATE_UNINSTALLED)
            {
                throw std::logic_error("Bundle " + symbolicName + " (location=" + location + ") is uninstalled");
            }
            if (state == Bundle::STATE_ACTIVE)
            {
                return;
            }

            if ((options & Bundle::START_TRANSIENT) == 0)
            {
                SetAutostartSetting(static_cast<int32_t>(options));
            }
        }

        ResolveAndActivate();
    }

    AnyMap const&
    BundlePrivate::GetHeaders() const
    {
        return bundleManifest.GetHeaders();
    }

    std::exception_ptr
    BundlePrivate::ActivateBundle()
    {
        // res is used to signal that start did not complete in a normal way
        std::exception_ptr res;
        auto const thisBundle = MakeBundle(this->shared_from_this());
        coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::BUNDLE_STARTING, thisBundle));

        auto const& headers = thisBundle.GetHeaders();
        Any bundleActivatorVal;
        if (headers.count(Constants::BUNDLE_ACTIVATOR) > 0)
        {
            bundleActivatorVal = headers.find(Constants::BUNDLE_ACTIVATOR)->second;
        }

        bool useActivator = false;
        if (!bundleActivatorVal.Empty())
        {
            try
            {
                useActivator = any_cast<bool>(bundleActivatorVal);
            }
            catch (BadAnyCastException const& ex)
            {
                std::string message("Failed to read 'bundle.activator' property. Expected type : ");
                message += typeid(useActivator).name();
                message += ", Found type : ";
                message += bundleActivatorVal.Type().name();
                coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_WARNING,
                                                                     thisBundle,
                                                                     message,
                                                                     std::make_exception_ptr(ex)));
            }
        }

        // Activator in the bundle is not called if 'bundle.activator' property
        // either does not exist or is set to false. If the property is set to true,
        // the actiavtor inside the bundle is called.
        if (useActivator)
        {
            try
            {
                if (coreCtx->validationFunc && (lib.GetFilePath() != util::GetExecutablePath())
                    && !coreCtx->validationFunc(thisBundle))
                {
                    CleanupAfterActivationFailure();
                    return std::make_exception_ptr(SecurityException {
                        "Bundle " + symbolicName + " (location=" + location + ") failed bundle validation.",
                        thisBundle });
                }
            }
            catch (...)
            {
                coreCtx->listeners.SendFrameworkEvent(
                    FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_WARNING,
                                   thisBundle,
                                   "The bundle validation function threw an exception",
                                   std::current_exception()));
                CleanupAfterActivationFailure();
                throw SecurityException { util::GetLastExceptionStr(), thisBundle };
            }

            try
            {
                void* libHandle = nullptr;
                if ((lib.GetFilePath() == util::GetExecutablePath()))
                {
                    libHandle = BundleUtils::GetExecutableHandle();
                }
                else
                {
                    if (!lib.IsLoaded())
                    {
                        coreCtx->logger->Log(logservice::SeverityLevel::LOG_INFO,
                                             "Loading shared library for Bundle " + symbolicName
                                                 + " (location=" + location + ")");
                        lib.Load(coreCtx->libraryLoadOptions);
                        coreCtx->logger->Log(logservice::SeverityLevel::LOG_INFO,
                                             "Finished loading shared library for Bundle " + symbolicName
                                                 + " (location=" + location + ")");
                    }
                    libHandle = lib.GetHandle();
                }

                auto ctx = bundleContext.Load();

                // save this bundle's context so that it can be accessible anywhere
                // from within this bundle's code.
                std::string set_bundle_context_func = US_STR(US_SET_CTX_PREFIX) + symbolicName;
                std::string set_bundle_context_err;
                BundleUtils::GetSymbol(SetBundleContext, libHandle, set_bundle_context_func, set_bundle_context_err);

                if (SetBundleContext)
                {
                    SetBundleContext(ctx.get());
                }
                else
                {
                    coreCtx->logger->Log(logservice::SeverityLevel::LOG_WARNING, set_bundle_context_err);
                }

                // get the create/destroy activator callbacks
                std::string create_activator_func = US_STR(US_CREATE_ACTIVATOR_PREFIX) + symbolicName;
                std::function<BundleActivator*(void)> createActivatorHook;
                std::string create_activator_err;
                BundleUtils::GetSymbol(createActivatorHook, libHandle, create_activator_func, create_activator_err);

                std::string destroy_activator_func = US_STR(US_DESTROY_ACTIVATOR_PREFIX) + symbolicName;
                std::string destroy_activator_err;
                BundleUtils::GetSymbol(destroyActivatorHook, libHandle, destroy_activator_func, destroy_activator_err);

                if (!createActivatorHook)
                {
                    coreCtx->logger->Log(logservice::SeverityLevel::LOG_ERROR, create_activator_err);
                    throw std::runtime_error("Bundle " + symbolicName + " (location=" + location
                                             + ") activator constructor not found");
                }
                if (!destroyActivatorHook)
                {
                    coreCtx->logger->Log(logservice::SeverityLevel::LOG_ERROR, destroy_activator_err);
                    throw std::runtime_error("Bundle " + symbolicName + " (location=" + location
                                             + ") activator destructor not found");
                }

                // get a BundleActivator instance
                bactivator = std::unique_ptr<BundleActivator, DestroyActivatorHook>(createActivatorHook(),
                                                                                    destroyActivatorHook);
                bactivator->Start(MakeBundleContext(ctx));
            }
            catch (std::system_error const& ex)
            {
                // SharedLibrary::Load(int flags) will throw a std::system_error when a shared library
                // fails to load. Creating a SharedLibraryException here to throw.
                res = std::make_exception_ptr(
                    cppmicroservices::SharedLibraryException(ex.code(), ex.what(), thisBundle));
            }
            catch (...)
            {
                coreCtx->logger->Log(logservice::SeverityLevel::LOG_INFO,
                                     "Failed to start Bundle " + symbolicName + " (location=" + location + ")",
                                     std::current_exception());
                res = std::make_exception_ptr(std::runtime_error("Bundle " + symbolicName + " (location= " + location
                                                                 + ") start failed: " + util::GetLastExceptionStr()));
            }
        }

        // activator.start() done
        // - normal -> state = active, started event
        // - exception from start() -> res = ex, start-failed clean-up
        // - unexpected state change (uninstall, refresh?):
        //   -> res = new exception
        // - start time-out -> res = new exception (not used?)

        // if start was aborted (uninstall or timeout), make sure
        // check aborted/state
        {
            std::string cause;
            if (static_cast<Aborted>(aborted.load()) == Aborted::YES)
            {
                if (Bundle::STATE_UNINSTALLED == GetBundleStateEnum())
                {
                    cause = "Bundle uninstalled during Start()";
                }
                else
                {
                    cause = "Bundle activator Start() time-out";
                }
            }
            else
            {
                aborted = static_cast<uint8_t>(Aborted::NO);
                if (Bundle::STATE_STARTING != GetBundleStateEnum())
                {
                    cause = "Bundle changed state because of refresh during Start()";
                }
            }
            if (!cause.empty())
            {
                res = std::make_exception_ptr(std::runtime_error("Bundle " + symbolicName + " (location= " + location
                                                                 + ") start failed: " + cause));
            }
        }

        if (res == nullptr)
        {
            SetLifecycleState(std::make_shared<BundleActiveState>());
            try
            {
                coreCtx->listeners.BundleChanged(
                    BundleEvent(BundleEvent::BUNDLE_STARTED, MakeBundle(this->shared_from_this())));
            }
            catch (cppmicroservices::SharedLibraryException const& ex)
            {
                res = std::make_exception_ptr(ex);
            }
            catch (cppmicroservices::SecurityException const& ex)
            {
                CleanupAfterActivationFailure();
                res = std::make_exception_ptr(ex);
            }
        }
        else
        {
            CleanupAfterActivationFailure();
        }
        return res;
    }

    void
    BundlePrivate::CleanupAfterActivationFailure()
    {
        SetLifecycleState(std::make_shared<BundleStoppingState>());
        coreCtx->listeners.BundleChanged(
            BundleEvent(BundleEvent::BUNDLE_STOPPING, MakeBundle(this->shared_from_this())));
        RemoveBundleResources();
        auto oldBundleContext = bundleContext.Exchange(std::shared_ptr<BundleContextPrivate>());
        if (oldBundleContext)
        {
            oldBundleContext->Invalidate();
        }
        SetLifecycleState(std::make_shared<BundleResolvedState>());
        coreCtx->listeners.BundleChanged(
            BundleEvent(BundleEvent::BUNDLE_STOPPED, MakeBundle(this->shared_from_this())));
    }

    BundlePrivate::BundlePrivate(CoreBundleContext* coreCtx)
        : coreCtx(coreCtx)
        , id(0)
        , location(Constants::SYSTEM_BUNDLE_LOCATION)
        , lifecycleState(std::make_shared<BundleInstalledState>())
        , barchive()
        , bundleDir(this->coreCtx->GetDataStorage(id))
        , bundleContext()
        , destroyActivatorHook(nullptr)
        , bactivator(nullptr, nullptr)
        , resolveFailException()
        , wasStarted(false)
        , aborted(static_cast<uint8_t>(Aborted::NONE))
        , symbolicName(Constants::SYSTEM_BUNDLE_SYMBOLICNAME)
        , version(CppMicroServices_VERSION_MAJOR, CppMicroServices_VERSION_MINOR, CppMicroServices_VERSION_PATCH)
        , timeStamp(std::chrono::steady_clock::now())
        , bundleManifest()
        , lib()
        , SetBundleContext(nullptr)
    {
    }

    BundlePrivate::BundlePrivate(CoreBundleContext* coreCtx, std::shared_ptr<BundleArchive> const& ba)
        : coreCtx(coreCtx)
        , id(ba->GetBundleId())
        , location(ba->GetBundleLocation())
        , lifecycleState(std::make_shared<BundleInstalledState>())
        , barchive(ba)
        , bundleDir(coreCtx->GetDataStorage(id))
        , bundleContext()
        , destroyActivatorHook(nullptr)
        , bactivator(nullptr, nullptr)
        , resolveFailException()
        , wasStarted(false)
        , aborted(static_cast<uint8_t>(Aborted::NONE))
        , symbolicName(ba->GetResourcePrefix())
        , version()
        , timeStamp(ba->GetLastModified())
        , bundleManifest(ba->GetInjectedManifest())
        , lib(location)
        , SetBundleContext(nullptr)
    {
        // Only take the time to read the manifest out of the BundleArchive file if we don't already have
        // a manifest.
        if (true == bundleManifest.GetHeaders().empty())
        {
            // Check if the bundle provides a manifest.json file and if yes, parse it.
            if (ba->IsValid())
            {
                auto manifestRes = ba->GetResource("/manifest.json");
                if (manifestRes)
                {
                    BundleResourceStream manifestStream(manifestRes);
                    try
                    {
                        bundleManifest.Parse(manifestStream);
                    }
                    catch (...)
                    {
                        throw std::runtime_error(std::string("Parsing of manifest.json for bundle ")
                                                 + ba->GetResourcePrefix() + " at " + location
                                                 + " failed: " + util::GetLastExceptionStr());
                    }
                    // It is unlikely that clients will access bundle resources
                    // if the only resource is the manifest file. On this assumption,
                    // close the open file handle to the zip file to improve performance
                    // and avoid exceeding OS open file handle limits.
                    if (OnlyContainsManifest(ba->GetResourceContainer()))
                    {
                        ba->GetResourceContainer()->CloseContainer();
                    }
                }
            }
        }

        // Check if we got version information and validate the version identifier
        if (bundleManifest.Contains(Constants::BUNDLE_VERSION))
        {
            Any versionAny = bundleManifest.GetValue(Constants::BUNDLE_VERSION);
            std::string errMsg;
            if (versionAny.Type() != typeid(std::string))
            {
                errMsg = std::string("The version identifier must be a string");
            }
            try
            {
                version = BundleVersion(versionAny.ToString());
            }
            catch (...)
            {
                errMsg = std::string("The version identifier is invalid: ") + util::GetLastExceptionStr();
            }

            if (!errMsg.empty())
            {
                throw std::invalid_argument(std::string("The Json value for ") + Constants::BUNDLE_VERSION
                                            + " for bundle " + symbolicName + " (location=" + location
                                            + ") is not valid: " + errMsg);
            }
        }

        if (!bundleManifest.Contains(Constants::BUNDLE_SYMBOLICNAME))
        {
            throw std::invalid_argument(Constants::BUNDLE_SYMBOLICNAME
                                        + " is not defined in the bundle manifest for bundle " + symbolicName
                                        + " (location=" + location + ").");
        }

        Any bsn(bundleManifest.GetValue(Constants::BUNDLE_SYMBOLICNAME));
        if (bsn.Empty() || bsn.ToStringNoExcept().empty())
        {
            throw std::invalid_argument(Constants::BUNDLE_SYMBOLICNAME + " is empty in the bundle manifest for bundle "
                                        + symbolicName + "(location=" + location + ").");
        }

        auto snbl = coreCtx->bundleRegistry.GetBundles(symbolicName, version);
        if (!snbl.empty())
        {
            throw std::invalid_argument("Bundle " + symbolicName + " (location=" + location
                                        + "), a bundle with same symbolic name and version " + "is already installed ("
                                        + symbolicName + ", " + version.ToString() + ")");
        }
    }

    BundlePrivate::~BundlePrivate() = default;

    void
    BundlePrivate::CheckUninstalled() const
    {
        if (GetBundleStateEnum() == Bundle::STATE_UNINSTALLED)
        {
            throw std::logic_error("Bundle " + symbolicName + " (location=" + location + ") is in UNINSTALLED state");
        }
    }

    void
    BundlePrivate::RemoveBundleResources()
    {
        coreCtx->listeners.RemoveAllListeners(bundleContext.Load());

        std::vector<ServiceRegistrationBase> srs;
        coreCtx->services.GetRegisteredByBundle(this, srs);
        for (auto& sr : srs)
        {
            try
            {
                sr.Unregister();
            }
            catch (std::logic_error const& /*ignore*/)
            {
                // Someone has unregistered the service after stop completed.
                // This should not occur, but we don't want get stuck in
                // an illegal state so we catch it.
            }
        }

        srs.clear();
        coreCtx->services.GetUsedByBundle(this, srs);
        for (std::vector<ServiceRegistrationBase>::const_iterator i = srs.begin(); i != srs.end(); ++i)
        {
            // wrap in try-catch to catch failures if service is already unregistered
            // if service is unregistered, all work in UngetService is already done by Unregister() previously
            try
            {
                auto ref = i->GetReference(std::string());
                ref.d.Load()->UngetService(this->shared_from_this(), false);
            }
            catch (...)
            {
                coreCtx->logger->Log(logservice::SeverityLevel::LOG_WARNING,
                                     "Some services already unregistered in Bundle " + symbolicName
                                         + " (location=" + location + ")",
                                     std::current_exception());
            }
        }
    }

    void
    BundlePrivate::Purge()
    {
        if (barchive->IsValid())
        {
            barchive->Purge();
        }
    }

    std::shared_ptr<BundleArchive>
    BundlePrivate::GetBundleArchive() const
    {
        return barchive;
    }

    void
    BundlePrivate::SetAutostartSetting(int32_t setting)
    {
        if (barchive->IsValid())
        {
            barchive->SetAutostartSetting(setting);
        }
    }

    int32_t
    BundlePrivate::GetAutostartSetting() const
    {
        return barchive->IsValid() ? barchive->GetAutostartSetting() : -1;
    }

    bool
    BundlePrivate::CompareAndSetState(std::shared_ptr<BundleLifecycleState>* expected,
                                      std::shared_ptr<BundleLifecycleState> desired)
    {
        return std::atomic_compare_exchange_strong(&lifecycleState, expected, desired);
    }

    std::shared_ptr<BundleLifecycleState>
    BundlePrivate::GetLifecycleState() const
    {
        return std::atomic_load(&lifecycleState);
    }

    Bundle::State
    BundlePrivate::GetBundleStateEnum() const
    {
        return std::atomic_load(&lifecycleState)->GetBundleState();
    }

    void
    BundlePrivate::SetLifecycleState(std::shared_ptr<BundleLifecycleState> newState)
    {
        std::atomic_store(&lifecycleState, std::move(newState));
    }

    std::exception_ptr
    BundlePrivate::DeactivateAndTransitionToResolved(std::shared_ptr<BundleLifecycleState> const& stoppingState)
    {
        std::exception_ptr savedException = DeactivateBundle();
        auto resolvedState = std::make_shared<BundleResolvedState>();
        auto expected = stoppingState;
        CompareAndSetState(&expected, resolvedState);
        coreCtx->listeners.BundleChanged({ BundleEvent::BUNDLE_STOPPED, MakeBundle(shared_from_this()) });
        return savedException;
    }

    void
    BundlePrivate::RemoveAndCleanupBundle()
    {
        coreCtx->bundleRegistry.Remove(location, id);

        // Transition to INSTALLED (for UNRESOLVED event), then to UNINSTALLED
        SetLifecycleState(std::make_shared<BundleInstalledState>());
        coreCtx->listeners.BundleChanged({ BundleEvent::BUNDLE_UNRESOLVED, MakeBundle(shared_from_this()) });
        bactivator = nullptr;

        SetLifecycleState(std::make_shared<BundleUninstalledState>());

        Purge();
        barchive->SetLastModified(std::chrono::steady_clock::now());

        if (!bundleDir.empty())
        {
            try
            {
                if (util::Exists(bundleDir))
                {
                    util::RemoveDirectoryRecursive(bundleDir);
                }
            }
            catch (...)
            {
                coreCtx->listeners.SendFrameworkEvent(
                    FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_WARNING,
                                   MakeBundle(shared_from_this()),
                                   std::string(),
                                   std::current_exception()));
            }
            bundleDir.clear();
        }
    }

    void
    BundlePrivate::ReportFrameworkError(std::exception_ptr exception)
    {
        if (exception)
        {
            try
            {
                std::rethrow_exception(exception);
            }
            catch (...)
            {
                coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_ERROR,
                                                                     MakeBundle(shared_from_this()),
                                                                     std::string(),
                                                                     std::current_exception()));
            }
        }
    }

    std::shared_ptr<BundlePrivate>
    GetPrivate(Bundle const& b)
    {
        return b.d;
    }
} // namespace cppmicroservices
