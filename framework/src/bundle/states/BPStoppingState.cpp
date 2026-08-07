#include "BPStoppingState.h"
#include "BPResolvedState.h"
#include "BPInstalledState.h"
#include "BPUninstalledState.h"
#include "BundlePrivate.h"
#include "BundleContextPrivate.h"
#include "CoreBundleContext.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleEvent.h"

namespace cppmicroservices
{
    uint32_t BPStoppingState::GetState(){
        return Bundle::STATE_STOPPING;
    };

    void BPStoppingState::Start(BundlePrivate& mgr, uint32_t options){

        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto stoppingState = std::make_shared<BPStoppingState>(std::move(fut)); 

        while(currState->GetState() == Bundle::STATE_STOPPING){
            if (mgr.CompareAndSetState(&currState, stoppingState)){
                currState->WaitForTransitionTask();

                auto frameworkBlock = CheckAndBlockFramework(mgr);
                SetAutostart(mgr, options);
                frameworkBlock.reset();
                transitionAction.set_value();

                throw std::runtime_error("Bundle " + mgr.symbolicName + " (location=" + mgr.location
                                            + "), start called from BundleActivator::Stop");
            }
        }
    }

    void BPStoppingState::Stop(BundlePrivate& mgr, uint32_t options){
        
        bool successfulTransition = false;
        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto resolvedState = std::make_shared<BPResolvedState>(std::move(fut)); 

        while(currState->GetState() == Bundle::STATE_STOPPING){
            if (mgr.CompareAndSetState(&currState, resolvedState)){
                currState->WaitForTransitionTask();

                SetAutostart(mgr, options);

                std::shared_ptr<BundleContextPrivate> ctx = mgr.bundleContext.Load();
                if (ctx)
                {
                    mgr.coreCtx->listeners.HooksBundleStopped(ctx);
                    mgr.RemoveBundleResources();
                    ctx->Invalidate();
                    mgr.bundleContext.Store(std::shared_ptr<BundleContextPrivate>());
                }
                mgr.coreCtx->listeners.BundleChanged({ BundleEvent::BUNDLE_STOPPED, MakeBundle(mgr.shared_from_this()) }); //listener stopp-ED event
                transitionAction.set_value();
                successfulTransition = true;
                break;
            }
        }

        if(!successfulTransition){
            LogDroppedTransition(mgr, "Stop()", Bundle::STATE_STOPPING, currState->GetState());
        }
    }

    void BPStoppingState::Uninstall(BundlePrivate& mgr){
        bool successfulTransition = false;
        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto installedState = std::make_shared<BPInstalledState>(std::move(fut)); 

        while(currState->GetState() == Bundle::STATE_STOPPING){
            if (mgr.CompareAndSetState(&currState, installedState)){
                currState->WaitForTransitionTask();
                std::shared_ptr<BundleContextPrivate> ctx = mgr.bundleContext.Load();
                if (ctx)
                {
                    mgr.coreCtx->listeners.HooksBundleStopped(ctx);
                    mgr.RemoveBundleResources();
                    ctx->Invalidate();
                    mgr.bundleContext.Store(std::shared_ptr<BundleContextPrivate>());
                }
                mgr.coreCtx->listeners.BundleChanged({ BundleEvent::BUNDLE_STOPPED, MakeBundle(mgr.shared_from_this()) }); //listener stopp-ED event
                transitionAction.set_value();
                successfulTransition = true;
                installedState->Uninstall(mgr);
                break;
            }
        }

        if(!successfulTransition){
            LogDroppedTransition(mgr, "Uninstall()", Bundle::STATE_STOPPING, currState->GetState());
        }
    }

} 
