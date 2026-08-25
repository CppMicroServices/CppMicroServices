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
    }

    void BPStoppingState::Start(BundlePrivate& mgr, uint32_t options){

        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto stoppingState = std::make_shared<BPStoppingState>(std::move(fut)); 

        while(observedState->GetState() == Bundle::STATE_STOPPING){
            observedState->WaitForTransitionTask();
            if (mgr.CompareAndSetState(&observedState, stoppingState)){
                TransitionCompletionGuard completeTransition(transitionAction);

                auto frameworkBlock = CheckAndBlockFramework(mgr);
                SetAutostart(mgr, options);
                frameworkBlock.reset();
                completeTransition.Complete();

                throw std::runtime_error("Bundle " + mgr.symbolicName + " (location=" + mgr.location
                                            + "), start called from BundleActivator::Stop");
            }
        }
    }

    namespace {
        void FinishBundleStop(BundlePrivate& mgr){
            std::shared_ptr<BundleContextPrivate> ctx = mgr.bundleContext.Load();
            if (ctx)
            {
                mgr.coreCtx->listeners.HooksBundleStopped(ctx);
                mgr.RemoveBundleResources();
                ctx->Invalidate();
                mgr.bundleContext.Store(std::shared_ptr<BundleContextPrivate>());
            }
            mgr.coreCtx->listeners.BundleChanged({ BundleEvent::BUNDLE_STOPPED, MakeBundle(mgr.shared_from_this()) }); 
        }
    }

    void BPStoppingState::Stop(BundlePrivate& mgr, uint32_t options){
        
        TransitionLogger transitionLogger(mgr, "Stop()", Bundle::STATE_STOPPING);
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto resolvedState = std::make_shared<BPResolvedState>(std::move(fut)); 

        while(observedState->GetState() == Bundle::STATE_STOPPING){
            observedState->WaitForTransitionTask();
            if (mgr.CompareAndSetState(&observedState, resolvedState)){
                TransitionCompletionGuard completeTransition(transitionAction);
                SetAutostart(mgr, options);
                FinishBundleStop(mgr);
                completeTransition.Complete();
                transitionLogger.TransitionSucceeded();
                break;
            }
        }

        transitionLogger.SetActualState(observedState);
    }

    void BPStoppingState::Uninstall(BundlePrivate& mgr){
        TransitionLogger transitionLogger(mgr, "Uninstall()", Bundle::STATE_STOPPING);
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto resolvedState = std::make_shared<BPResolvedState>(std::move(fut)); 

        while(observedState->GetState() == Bundle::STATE_STOPPING){
            observedState->WaitForTransitionTask();
            if (mgr.CompareAndSetState(&observedState, resolvedState)){
                TransitionCompletionGuard completeTransition(transitionAction);
                FinishBundleStop(mgr);
                completeTransition.Complete();
                transitionLogger.TransitionSucceeded();
                resolvedState->Uninstall(mgr);
                break;
            }
        }

        transitionLogger.SetActualState(observedState);

    }

    void BPStoppingState::StartFailed(BundlePrivate& mgr){
        
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto resolvedState = std::make_shared<BPResolvedState>(std::move(fut)); 

        if (mgr.CompareAndSetState(&observedState, resolvedState)){
            TransitionCompletionGuard completeTransition(transitionAction);
            mgr.RemoveBundleResources();
            auto oldBundleContext = mgr.bundleContext.Exchange(std::shared_ptr<BundleContextPrivate>());
            if (oldBundleContext)
            {
                oldBundleContext->Invalidate();
            }
            mgr.coreCtx->listeners.BundleChanged({ BundleEvent::BUNDLE_STOPPED, MakeBundle(mgr.shared_from_this()) }); 
            completeTransition.Complete();
        }
    }
}

