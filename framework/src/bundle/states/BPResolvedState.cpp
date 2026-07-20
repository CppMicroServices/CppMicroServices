#include "BPResolvedState.h"
#include "BPStartingState.h"
#include "BPStoppingState.h"
#include "BPInstalledState.h"
#include "BundlePrivate.h"
#include "BundleContextPrivate.h"
#include "CoreBundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/util/Error.h"
#include "cppmicroservices/util/FileSystem.h"
#include "cppmicroservices/util/String.h"
#include "cppmicroservices/SharedLibraryException.h"
#include "BundleUtils.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/AnyMap.h"

namespace cppmicroservices
{
    // void StartFailed(BundlePrivate& mgr, std::shared_ptr<BundlePrivateState> expectedState);


    void BPResolvedState::Start(BundlePrivate& mgr, uint32_t options){
        CheckFrameworkHasStopped(mgr);
        SetAutostart(mgr, options);

        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto startingState = std::make_shared<BPStartingState>(std::move(fut));

        while(currState->GetState() == Bundle::STATE_RESOLVED){
            if (mgr.CompareAndSetState(&currState, startingState)){
                currState->WaitForTransitionTask();
                std::__exception_ptr::exception_ptr e;                   
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

                // Activator in the bundle is not called if 'bundle.activator' property
                // either does not exist or is set to false. If the property is set to true,
                // the actiavtor inside the bundle is called.
                if (useActivator)
                {
                    //validate bundle
                    try
                    {
                        if (mgr.coreCtx->validationFunc && (mgr.lib.GetFilePath() != util::GetExecutablePath())
                            && !mgr.coreCtx->validationFunc(thisBundle))
                        {
                            e = std::make_exception_ptr(SecurityException {
                                "Bundle " + mgr.symbolicName + " (location=" + mgr.location + ") failed bundle validation.",
                                thisBundle });
                        }
                    }
                    catch (...)
                    {
                        mgr.coreCtx->listeners.SendFrameworkEvent(
                            FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_WARNING,
                                        thisBundle,
                                        "The bundle validation function threw an exception",
                                        std::current_exception()));
                        e = std::make_exception_ptr(SecurityException { util::GetLastExceptionStr(), thisBundle });
                    }

                    if(e == nullptr){
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

                            // save this bundle's context so that it can be accessible anywhere
                            // from within this bundle's code.
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

                            // get the create/destroy activator callbacks
                            // these are hooks that need to be setup and then they will automatically run?
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

                            // get a BundleActivator instance
                            mgr.bactivator = std::unique_ptr<BundleActivator, BundlePrivate::DestroyActivatorHook>(createActivatorHook(),
                                                                                                mgr.destroyActivatorHook);
                            mgr.bactivator->Start(MakeBundleContext(ctx)); //run bactivator start code
                        }
                        catch (std::system_error const& ex)
                        {
                            // SharedLibrary::Load(int flags) will throw a std::system_error when a shared library
                            // fails to load. Creating a SharedLibraryException here to throw.
                            e = std::make_exception_ptr(
                                cppmicroservices::SharedLibraryException(ex.code(), ex.what(), thisBundle));
                        }
                        catch (...)
                        {
                            mgr.coreCtx->logger->Log(logservice::SeverityLevel::LOG_INFO,
                                                "Failed to start Bundle " + mgr.symbolicName + " (location=" + mgr.location + ")",
                                                std::current_exception());
                            e = std::make_exception_ptr(std::runtime_error("Bundle " + mgr.symbolicName + " (location= " + mgr.location
                                                                            + ") start failed: " + util::GetLastExceptionStr()));
                        }
                    }
                }
                
                if (e == nullptr)
                {
                    transitionAction.set_value();
                    startingState->Start(mgr, options);

                }
                else
                {
                    transitionAction.set_value();
                    startingState->Stop(mgr, options);
                    std::rethrow_exception(e);
                }
            }
        }

    }

    void BPResolvedState::Uninstall(BundlePrivate& mgr){

        // auto currState = shared_from_this(); 
        // std::promise<void> transitionAction; 
        // auto fut = transitionAction.get_future();
        // auto installedState = std::make_shared<BPInstalledState>(std::move(fut));

        // while(currState->GetState() != Bundle::STATE_INSTALLED){
        //     if (mgr.CompareAndSetState(&currState, installedState))
        //     {
        //         currState->WaitForTransitionTask();
        //         mgr.coreCtx->listeners.BundleChanged(
        //                 { BundleEvent::BUNDLE_UNRESOLVED, MakeBundle(mgr.shared_from_this()) });
        //         transitionAction.set_value();
        //         installedState->Uninstall(mgr);
        //         break;
        //     }
        // }

        auto currState = shared_from_this(); 
        auto installedState = std::make_shared<BPInstalledState>();

        while(currState->GetState() != Bundle::STATE_INSTALLED){
            if (mgr.CompareAndSetState(&currState, installedState))
            {
                installedState->Uninstall(mgr);
                break;
            }
        }
        
    };

    
} 
