#include "BPIdleState.h"
#include "BPResolvedState.h"
#include "BPActiveState.h"
#include "BundlePrivate.h"
#include "BundleContextPrivate.h"
#include "CoreBundleContext.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/util/Error.h"
#include "cppmicroservices/util/FileSystem.h"
#include "cppmicroservices/util/String.h"
#include "BundleUtils.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/SharedLibraryException.h"


namespace cppmicroservices
{
    void BPIdleState::StartFromIdle(BundlePrivate& mgr){
        mgr.SetStateValue(Bundle::STATE_STARTING);
        std::shared_ptr<BundleContextPrivate> null_expected;
        std::shared_ptr<BundleContextPrivate> ctx(new BundleContextPrivate(&mgr));
        mgr.bundleContext.CompareExchange(null_expected, ctx);
        auto const thisBundle = MakeBundle(mgr.shared_from_this());
        mgr.coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::BUNDLE_STARTING, thisBundle));
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
                mgr.coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_WARNING,
                                                                    thisBundle,
                                                                    message,
                                                                    std::make_exception_ptr(ex)));
            }
        }

        if (useActivator)
        {
            bool failedValidation;
            try
            {
                failedValidation = (mgr.coreCtx->validationFunc && (mgr.lib.GetFilePath() != util::GetExecutablePath())
                    && !mgr.coreCtx->validationFunc(thisBundle));
            }
            catch (...)
            {
                mgr.coreCtx->listeners.SendFrameworkEvent(
                    FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_WARNING,
                                thisBundle,
                                "The bundle validation function threw an exception",
                                std::current_exception()));
                StartFailed(mgr);
                throw (SecurityException { util::GetLastExceptionStr(), thisBundle });
            }

            if(failedValidation){
                StartFailed(mgr);
                throw (SecurityException {
                    "Bundle " + mgr.symbolicName + " (location=" + mgr.location + ") failed bundle validation.",
                    thisBundle });
            }

            try
            {
                void* libHandle = nullptr;
                if ((mgr.lib.GetFilePath() == util::GetExecutablePath()))
                {
                    libHandle = BundleUtils::GetExecutableHandle();
                }
                else
                {
                    if (!mgr.lib.IsLoaded())
                    {
                        mgr.coreCtx->logger->Log(logservice::SeverityLevel::LOG_INFO,
                                            "Loading shared library for Bundle " + mgr.symbolicName
                                                + " (location=" + mgr.location + ")");
                        mgr.lib.Load(mgr.coreCtx->libraryLoadOptions);
                        mgr.coreCtx->logger->Log(logservice::SeverityLevel::LOG_INFO,
                                            "Finished loading shared library for Bundle " + mgr.symbolicName
                                                + " (location=" + mgr.location + ")");
                    }
                    libHandle = mgr.lib.GetHandle();
                }

                auto ctx = mgr.bundleContext.Load();

                std::string set_bundle_context_func = US_STR(US_SET_CTX_PREFIX) + mgr.symbolicName;
                std::string set_bundle_context_err;
                BundleUtils::GetSymbol(mgr.SetBundleContext, libHandle, set_bundle_context_func, set_bundle_context_err);

                if (mgr.SetBundleContext)
                {
                    mgr.SetBundleContext(ctx.get());
                }
                else
                {
                    mgr.coreCtx->logger->Log(logservice::SeverityLevel::LOG_WARNING, set_bundle_context_err);
                }

                std::string create_activator_func = US_STR(US_CREATE_ACTIVATOR_PREFIX) + mgr.symbolicName;
                std::function<BundleActivator*(void)> createActivatorHook;
                std::string create_activator_err;
                BundleUtils::GetSymbol(createActivatorHook, libHandle, create_activator_func, create_activator_err);

                std::string destroy_activator_func = US_STR(US_DESTROY_ACTIVATOR_PREFIX) + mgr.symbolicName;
                std::string destroy_activator_err;
                BundleUtils::GetSymbol(mgr.destroyActivatorHook, libHandle, destroy_activator_func, destroy_activator_err);

                if (!createActivatorHook)
                {
                    mgr.coreCtx->logger->Log(logservice::SeverityLevel::LOG_ERROR, create_activator_err);
                    throw std::runtime_error("Bundle " + mgr.symbolicName + " (location=" + mgr.location
                                            + ") activator constructor not found");
                }
                if (!mgr.destroyActivatorHook)
                {
                    mgr.coreCtx->logger->Log(logservice::SeverityLevel::LOG_ERROR, destroy_activator_err);
                    throw std::runtime_error("Bundle " + mgr.symbolicName + " (location=" + mgr.location
                                            + ") activator destructor not found");
                }

                mgr.bactivator = std::unique_ptr<BundleActivator, BundlePrivate::DestroyActivatorHook>(createActivatorHook(),
                                                                                    mgr.destroyActivatorHook);
                mgr.bactivator->Start(MakeBundleContext(ctx));
            }
            catch (std::system_error const& ex)
            {
                StartFailed(mgr);
                throw (cppmicroservices::SharedLibraryException(ex.code(), ex.what(), thisBundle));
            }
            catch (...)
            {
                mgr.coreCtx->logger->Log(logservice::SeverityLevel::LOG_INFO,
                                    "Failed to start Bundle " + mgr.symbolicName + " (location=" + mgr.location + ")",
                                    std::current_exception());
                StartFailed(mgr);
                throw (std::runtime_error("Bundle " + mgr.symbolicName + " (location= " + mgr.location
                                                                + ") start failed: " + util::GetLastExceptionStr()));
            }
        }

        mgr.SetStateValue(Bundle::STATE_ACTIVE);
        try
        {
            mgr.coreCtx->listeners.BundleChanged(
                BundleEvent(BundleEvent::BUNDLE_STARTED, MakeBundle(mgr.shared_from_this())));

        }
        catch (cppmicroservices::SecurityException const& ex)
        {
            StartFailed(mgr);
            throw;
        }
    }

    void BPIdleState::StartFailed(BundlePrivate& mgr){
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto newState = std::make_shared<BPResolvedState>(std::move(fut));
        if(mgr.CompareAndSetState(&observedState, newState)){ 
            TransitionCompletionGuard completeTransition(transitionAction);
            mgr.SetStateValue(Bundle::STATE_STOPPING);
            mgr.coreCtx->listeners.BundleChanged(
                BundleEvent(BundleEvent::BUNDLE_STOPPING, MakeBundle(mgr.shared_from_this())));
            mgr.RemoveBundleResources();
            auto oldBundleContext = mgr.bundleContext.Exchange(std::shared_ptr<BundleContextPrivate>());
            if (oldBundleContext)
            {
                oldBundleContext->Invalidate();
            }
            mgr.SetStateValue(Bundle::STATE_RESOLVED);
            mgr.coreCtx->listeners.BundleChanged({ BundleEvent::BUNDLE_STOPPED, MakeBundle(mgr.shared_from_this()) }); 
        }
    }

} 
