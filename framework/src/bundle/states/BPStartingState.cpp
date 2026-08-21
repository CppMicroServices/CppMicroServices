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
            if (mgr.CompareAndSetState(&observedState, activeState)){
                observedState->WaitForTransitionTask();
                
                auto frameworkBlock = CheckAndBlockFramework(mgr);
                SetAutostart(mgr, options);

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
                    frameworkBlock.reset();
                    transitionAction.set_value();
                    activeState->Stop(mgr, options);
                    throw ex;
                }

                frameworkBlock.reset();
                transitionAction.set_value();
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
            if (mgr.CompareAndSetState(&observedState, stoppingState)){
                observedState->WaitForTransitionTask();
                SetAutostart(mgr, options);
                StopStartingBundle(mgr);
                transitionAction.set_value();
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
            if (mgr.CompareAndSetState(&observedState, stoppingState)){
                observedState->WaitForTransitionTask();
                StopStartingBundle(mgr);
                transitionAction.set_value();
                transitionLogger.TransitionSucceeded();
                stoppingState->Uninstall(mgr);
                break;
            }
        }

        transitionLogger.SetActualState(observedState);
    }

} 
