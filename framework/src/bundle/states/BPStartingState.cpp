#include "BPStartingState.h"
#include "BPStoppingState.h"
#include "BPActiveState.h"
#include "BundlePrivate.h"
#include "CoreBundleContext.h"
#include "BundleContextPrivate.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/SharedLibraryException.h"
#include "cppmicroservices/util/Error.h"
#include "cppmicroservices/util/FileSystem.h"
#include "cppmicroservices/util/String.h"
#include "BundleUtils.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/Constants.h"

namespace cppmicroservices
{
    
    uint32_t BPStartingState::GetState(){
        return Bundle::STATE_STARTING;
    };

    void BPStartingState::Start(BundlePrivate& mgr, uint32_t options){

        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto activeState = std::make_shared<BPActiveState>(std::move(fut)); 

        while(currState->GetState() == Bundle::STATE_STARTING){
            if (mgr.CompareAndSetState(&currState, activeState)){
                currState->WaitForTransitionTask();
                
                CheckFrameworkHasStopped(mgr);
                SetAutostart(mgr, options);

                auto const thisBundle = MakeBundle(mgr.shared_from_this());
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
                    bool valid = true;
                    try
                    {
                        if (mgr.coreCtx->validationFunc && (mgr.lib.GetFilePath() != util::GetExecutablePath())
                            && !mgr.coreCtx->validationFunc(thisBundle))
                        {
                            valid = false;
                        }
                    }
                    catch (...)
                    {
                        mgr.coreCtx->listeners.SendFrameworkEvent(
                            FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_WARNING,
                                        thisBundle,
                                        "The bundle validation function threw an exception",
                                        std::current_exception()));
                        transitionAction.set_value();
                        activeState->Stop(mgr, options);
                        throw (SecurityException { util::GetLastExceptionStr(), thisBundle });
                    }

                    if(!valid){
                        transitionAction.set_value();
                        activeState->Stop(mgr, options);
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
                        transitionAction.set_value();
                        activeState->Stop(mgr, options);
                        throw (cppmicroservices::SharedLibraryException(ex.code(), ex.what(), thisBundle));
                    }
                    catch (...)
                    {
                        mgr.coreCtx->logger->Log(logservice::SeverityLevel::LOG_INFO,
                                            "Failed to start Bundle " + mgr.symbolicName + " (location=" + mgr.location + ")",
                                            std::current_exception());
                        transitionAction.set_value();
                        activeState->Stop(mgr, options);
                        throw (std::runtime_error("Bundle " + mgr.symbolicName + " (location= " + mgr.location
                                                                        + ") start failed: " + util::GetLastExceptionStr()));
                    }
                }

                try
                {
                    mgr.coreCtx->listeners.BundleChanged(
                        BundleEvent(BundleEvent::BUNDLE_STARTED, MakeBundle(mgr.shared_from_this())));
                    
                }
                catch (cppmicroservices::SharedLibraryException const& ex)
                {
                    transitionAction.set_value();
                    throw ex;
                }
                catch (cppmicroservices::SecurityException const& ex)
                {
                    transitionAction.set_value();
                    activeState->Stop(mgr, options);
                    throw ex;
                }

                transitionAction.set_value();
                break;
            }
        }
    }

    void BPStartingState::Stop(BundlePrivate& mgr, uint32_t options){
        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto stoppingState = std::make_shared<BPStoppingState>(std::move(fut));

        while(currState->GetState() == Bundle::STATE_STARTING){
            if (mgr.CompareAndSetState(&currState, stoppingState)){
                currState->WaitForTransitionTask();
                
                SetAutostart(mgr, options);

                mgr.coreCtx->listeners.BundleChanged(
                    BundleEvent(BundleEvent::BUNDLE_STOPPING, MakeBundle(mgr.shared_from_this())));
                    
                transitionAction.set_value();
                stoppingState->Stop(mgr, options);
                break;
            }
        }
    }

    void BPStartingState::Uninstall(BundlePrivate& mgr){
        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto stoppingState = std::make_shared<BPStoppingState>(std::move(fut));

        while(currState->GetState() == Bundle::STATE_STARTING){
            if (mgr.CompareAndSetState(&currState, stoppingState)){
                currState->WaitForTransitionTask();
                mgr.coreCtx->listeners.BundleChanged(
                    BundleEvent(BundleEvent::BUNDLE_STOPPING, MakeBundle(mgr.shared_from_this())));
                transitionAction.set_value();
                stoppingState->Uninstall(mgr);
                break;
            }
        }
    }

} 
