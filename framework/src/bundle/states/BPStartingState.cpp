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

        TransitionLogger transitionLogger(mgr, "Start()", Bundle::STATE_STARTING);
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto activeState = std::make_shared<BPActiveState>(std::move(fut)); 

        while(observedState->GetState() == Bundle::STATE_STARTING){
            observedState->WaitForTransitionTask();
            if (mgr.CompareAndSetState(&observedState, activeState)){
                TransitionCompletionGuard completeTransition(transitionAction);
                
                auto frameworkBlock = CheckAndBlockFramework(mgr);
                SetAutostart(mgr, options);

                try
                {
                    mgr.coreCtx->listeners.BundleChanged(
                        BundleEvent(BundleEvent::BUNDLE_STARTED, MakeBundle(mgr.shared_from_this())));
                    
                }
                catch (cppmicroservices::SecurityException const& ex)
                {
                    frameworkBlock.reset();
                    activeState->StartFailed(mgr);
                    completeTransition.Complete();
                    throw ex;
                }

                frameworkBlock.reset();
                completeTransition.Complete();
                transitionLogger.TransitionSucceeded();
                break;
            }
        }

        transitionLogger.SetActualState(observedState);
    }

    namespace{
        void StopStartingBundle(BundlePrivate& mgr){
            mgr.coreCtx->listeners.BundleChanged(
                BundleEvent(BundleEvent::BUNDLE_STOPPING, MakeBundle(mgr.shared_from_this())));
        }
    }

    void BPStartingState::Stop(BundlePrivate& mgr, uint32_t options){
        TransitionLogger transitionLogger(mgr, "Stop()", Bundle::STATE_STARTING);
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto stoppingState = std::make_shared<BPStoppingState>(std::move(fut));

        while(observedState->GetState() == Bundle::STATE_STARTING){
            observedState->WaitForTransitionTask();
            if (mgr.CompareAndSetState(&observedState, stoppingState)){
                TransitionCompletionGuard completeTransition(transitionAction);
                SetAutostart(mgr, options);
                StopStartingBundle(mgr);
                completeTransition.Complete();
                transitionLogger.TransitionSucceeded();
                stoppingState->Stop(mgr, options);
                break;
            }
        }

        transitionLogger.SetActualState(observedState);
    }

    void BPStartingState::Uninstall(BundlePrivate& mgr){
        TransitionLogger transitionLogger(mgr, "Uninstall()", Bundle::STATE_STARTING);
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto stoppingState = std::make_shared<BPStoppingState>(std::move(fut));

        while(observedState->GetState() == Bundle::STATE_STARTING){
            observedState->WaitForTransitionTask();
            if (mgr.CompareAndSetState(&observedState, stoppingState)){
                TransitionCompletionGuard completeTransition(transitionAction);
                StopStartingBundle(mgr);
                completeTransition.Complete();
                transitionLogger.TransitionSucceeded();
                stoppingState->Uninstall(mgr);
                break;
            }
        }

        transitionLogger.SetActualState(observedState);
    }

    void BPStartingState::StartFailed(BundlePrivate& mgr){
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto stoppingState = std::make_shared<BPStoppingState>(std::move(fut));

        while(observedState->GetState() == Bundle::STATE_STARTING){
            observedState->WaitForTransitionTask();
            if (mgr.CompareAndSetState(&observedState, stoppingState)){
                TransitionCompletionGuard completeTransition(transitionAction);
                StopStartingBundle(mgr);
                completeTransition.Complete();
                stoppingState->StartFailed(mgr);
                break;
            }
        }
    }

} 
