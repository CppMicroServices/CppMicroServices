#include "BPRunningState.h"
#include "BPResolvedState.h"
#include "BPStoppingState.h"
#include "BPUninstalledState.h"
#include "BundlePrivate.h"
#include "cppmicroservices/BundleActivator.h"
#include "CoreBundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/FrameworkEvent.h"

#include "cppmicroservices/util/Error.h"
#include "cppmicroservices/util/FileSystem.h"
#include "cppmicroservices/util/String.h"
#include "BundleContextPrivate.h"
#include "cppmicroservices/util/FileSystem.h"
#include <chrono>


namespace cppmicroservices
{
    std::exception_ptr StopHelper(BundlePrivate& mgr);

    void BPRunningState::Stop(BundlePrivate& mgr, uint32_t options){
        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto stoppingState = std::make_shared<BPStoppingState>(std::move(fut));

        while(currState->GetState() != Bundle::STATE_STOPPING){
            if (mgr.CompareAndSetState(&currState, stoppingState)){
                currState->WaitForTransitionTask();
                std::exception_ptr res = StopHelper(mgr);
                
                if (res){
                    auto state = std::dynamic_pointer_cast<BundlePrivateState>(stoppingState);
                    mgr.CompareAndSetState(&state, std::make_shared<BPResolvedState>());
                    transitionAction.set_value();
                    std::rethrow_exception(res);
                }
                transitionAction.set_value();
                stoppingState->Stop(mgr, options);
                break;
            }
        }
    }


    void BPRunningState::Uninstall(BundlePrivate& mgr){
        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto stoppingState = std::make_shared<BPStoppingState>(std::move(fut));

        while(currState->GetState() != Bundle::STATE_STOPPING){
            if (mgr.CompareAndSetState(&currState, stoppingState)){
                currState->WaitForTransitionTask();

                std::exception_ptr res;
                res = StopHelper(mgr);
                // try {
                //     res = StopHelper(mgr);
                // } 
                // catch (...){
                //     mgr.SetStateInstalled(false);
                //     res = std::current_exception();
                // }

                if (res){
                    mgr.coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_ERROR,
                                                                                MakeBundle(mgr.shared_from_this()),
                                                                                std::string(),
                                                                                res));
                }
                
                transitionAction.set_value();
                stoppingState->Uninstall(mgr);
                break;

            }
        }
    }

    std::exception_ptr StopHelper(BundlePrivate& mgr){
        std::exception_ptr res;
        mgr.coreCtx->listeners.BundleChanged(
            BundleEvent(BundleEvent::BUNDLE_STOPPING, MakeBundle(mgr.shared_from_this()))); //stop-ING listener event

        if (mgr.wasStarted && mgr.bactivator != nullptr) //if coming from active, and there is bactivator code
        {
            try
            {
                mgr.bactivator->Stop(MakeBundleContext(mgr.bundleContext.Load())); //run bactivator stop code
            }
            catch (...)
            {
                res = std::make_exception_ptr(
                    std::runtime_error("Bundle " + mgr.symbolicName + " (location=" + mgr.location
                                    + "), BundleActivator::Stop() failed: " + util::GetLastExceptionStr()));
            }
            mgr.bactivator = nullptr;
        }

        std::shared_ptr<BundleContextPrivate> ctx = mgr.bundleContext.Load();
        if (ctx)
        {
            mgr.coreCtx->listeners.HooksBundleStopped(ctx); //listener event, looks different from the other ones though 
            mgr.RemoveBundleResources();
            ctx->Invalidate();
            mgr.bundleContext.Store(std::shared_ptr<BundleContextPrivate>());
        }
        
        return res;
    }

}; 
