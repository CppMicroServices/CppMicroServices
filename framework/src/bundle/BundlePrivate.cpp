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

#include "states/BPStartingState.h"
#include "states/BPStoppingState.h"
#include "states/BPResolvedState.h"
#include "states/BPInstalledState.h"
#include "states/BPActiveState.h"
#include "states/BPUninstalledState.h"

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
        GetStateObj()->Stop(*this, options);
    }

    void
    BundlePrivate::Start(uint32_t options)
    {
        GetStateObj()->Start(*this, options);
    }


    void
    BundlePrivate::Uninstall()
    {
        GetStateObj()->Uninstall(*this);
    }

    bool
    BundlePrivate::CompareAndSetState(std::shared_ptr<BundlePrivateState>* expectedState,
                                                    std::shared_ptr<BundlePrivateState> desiredState)
    {
        return std::atomic_compare_exchange_strong(&state, expectedState, desiredState);
    }

    AnyMap const&
    BundlePrivate::GetHeaders() const
    {
        return bundleManifest.GetHeaders();
    }

    std::shared_ptr<BundlePrivateState>
    BundlePrivate::GetStateObj() const
    {
        return std::atomic_load(&state);
    }

    uint32_t BundlePrivate::GetState() const
    {
        return GetStateObj()->GetState();
    }


    std::string
    BundlePrivate::GetLocation() const
    {
        return location;
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

        auto installedState = std::make_shared<BPInstalledState>();
        auto observedState = GetStateObj();
        CompareAndSetState(&observedState, installedState);
        // state = Bundle::STATE_INSTALLED;

        if (sendEvent)
        {
            coreCtx->listeners.BundleChanged({ BundleEvent::BUNDLE_RESOLVED, MakeBundle(this->shared_from_this()) });
        }
        return;
    }

    BundlePrivate::BundlePrivate(CoreBundleContext* coreCtx)
        : coreCtx(coreCtx)
        , id(0)
        , location(Constants::SYSTEM_BUNDLE_LOCATION)
        , barchive()
        , bundleDir(this->coreCtx->GetDataStorage(id))
        , bundleContext()
        , destroyActivatorHook(nullptr)
        , bactivator(nullptr, nullptr)
        , resolveFailException()
        , symbolicName(Constants::SYSTEM_BUNDLE_SYMBOLICNAME)
        , version(CppMicroServices_VERSION_MAJOR, CppMicroServices_VERSION_MINOR, CppMicroServices_VERSION_PATCH)
        , timeStamp(std::chrono::steady_clock::now())
        , bundleManifest()
        , lib()
        , SetBundleContext(nullptr)
        , state(std::make_shared<BPInstalledState>())
    {
    }

    BundlePrivate::BundlePrivate(CoreBundleContext* coreCtx, std::shared_ptr<BundleArchive> const& ba)
        : coreCtx(coreCtx)
        , id(ba->GetBundleId())
        , location(ba->GetBundleLocation())
        , barchive(ba)
        , bundleDir(coreCtx->GetDataStorage(id))
        , bundleContext()
        , destroyActivatorHook(nullptr)
        , bactivator(nullptr, nullptr)
        , resolveFailException()
        , symbolicName(ba->GetResourcePrefix())
        , version()
        , timeStamp(ba->GetLastModified())
        , bundleManifest(ba->GetInjectedManifest())
        , lib(location)
        , SetBundleContext(nullptr)
        , state(std::make_shared<BPInstalledState>())
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
        if (GetState() == Bundle::STATE_UNINSTALLED)
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

    std::shared_ptr<BundlePrivate>
    GetPrivate(Bundle const& b)
    {
        return b.d;
    }
} // namespace cppmicroservices
