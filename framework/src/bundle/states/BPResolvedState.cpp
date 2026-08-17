#include "BPResolvedState.h"
#include "BPStartingState.h"
#include "BPInstalledState.h"
#include "BundlePrivate.h"
#include "BundleContextPrivate.h"
#include "CoreBundleContext.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleEvent.h"

namespace cppmicroservices
{

    uint32_t BPResolvedState::GetState(){
        return Bundle::STATE_RESOLVED;
    };

    void BPResolvedState::Start(BundlePrivate& mgr, uint32_t options){
        TransitionLogger transitionLogger(mgr, "Start()", Bundle::STATE_RESOLVED);
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto startingState = std::make_shared<BPStartingState>(std::move(fut));

        while(observedState->GetState() == Bundle::STATE_RESOLVED){
            if (mgr.CompareAndSetState(&observedState, startingState)){
                observedState->WaitForTransitionTask();

                auto frameworkBlock = CheckAndBlockFramework(mgr);
                SetAutostart(mgr, options);

                std::shared_ptr<BundleContextPrivate> null_expected;
                std::shared_ptr<BundleContextPrivate> ctx(new BundleContextPrivate(&mgr));
                mgr.bundleContext.CompareExchange(null_expected, ctx);
                auto const thisBundle = MakeBundle(mgr.shared_from_this());
                mgr.coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::BUNDLE_STARTING, thisBundle));

                frameworkBlock.reset();
                transitionAction.set_value();
                startingState->Start(mgr, options);
                transitionLogger.TransitionSucceeded();
                break;
            }
        }

        transitionLogger.SetActualState(observedState);
    }

    void BPResolvedState::Stop(BundlePrivate& mgr, uint32_t options){
        TransitionLogger transitionLogger(mgr, "Stop()", Bundle::STATE_RESOLVED);
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto resolvedState = std::make_shared<BPResolvedState>(std::move(fut)); 

        while(observedState->GetState() == Bundle::STATE_RESOLVED){
            if (mgr.CompareAndSetState(&observedState, resolvedState)){
                observedState->WaitForTransitionTask(); 
                SetAutostart(mgr, options);
                transitionAction.set_value();
                transitionLogger.TransitionSucceeded();
                break;
            }
        }

        transitionLogger.SetActualState(observedState);
    }

    void BPResolvedState::Uninstall(BundlePrivate& mgr){
        TransitionLogger transitionLogger(mgr, "Uninstall()", Bundle::STATE_RESOLVED);
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto installedState = std::make_shared<BPInstalledState>(std::move(fut));

        while(observedState->GetState() == Bundle::STATE_RESOLVED){
            if (mgr.CompareAndSetState(&observedState, installedState))
            {
                observedState->WaitForTransitionTask();
                transitionAction.set_value();
                transitionLogger.TransitionSucceeded();
                installedState->Uninstall(mgr);
                break;
            }
        }

        transitionLogger.SetActualState(observedState);
    }

    
} 
