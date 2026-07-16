#include "BPStoppingState.h"
#include "BPResolvedState.h"
#include "BPUninstalledState.h"
#include "BundlePrivate.h"
#include "CoreBundleContext.h"
#include "cppmicroservices/BundleEvent.h"

namespace cppmicroservices
{

    void BPStoppingState::Start(BundlePrivate& mgr, uint32_t options){
        CheckFrameworkHasStopped(mgr);
        SetAutostart(mgr, options);
        throw std::runtime_error("Bundle " + mgr.symbolicName + " (location=" + mgr.location
                                    + "), start called from BundleActivator::Stop");
    }

    void BPStoppingState::Stop(BundlePrivate& mgr, uint32_t options){
        US_UNUSED(options);
        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto resolvedState = std::make_shared<BPResolvedState>(std::move(fut)); 

        while(currState->GetState() == Bundle::STATE_STOPPING){
            if (mgr.CompareAndSetState(&currState, resolvedState)){
                currState->WaitForTransitionTask();
                mgr.coreCtx->listeners.BundleChanged({ BundleEvent::BUNDLE_STOPPED, MakeBundle(mgr.shared_from_this()) }); //listener stopp-ED event
                transitionAction.set_value();
                break;
            }
        }
    }

    void BPStoppingState::Uninstall(BundlePrivate& mgr){
        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto resolvedState = std::make_shared<BPResolvedState>(std::move(fut)); 

        while(currState->GetState() == Bundle::STATE_STOPPING){
            if (mgr.CompareAndSetState(&currState, resolvedState)){
                currState->WaitForTransitionTask();
                mgr.coreCtx->listeners.BundleChanged({ BundleEvent::BUNDLE_STOPPED, MakeBundle(mgr.shared_from_this()) }); //listener stopp-ED event
                transitionAction.set_value();
                resolvedState->Uninstall(mgr);
                break;
            }
        }
    }

} 
