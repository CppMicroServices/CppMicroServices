#include "BPInstalledState.h"
#include "BPResolvedState.h"
#include "CoreBundleContext.h"
#include "BundlePrivate.h"
#include "cppmicroservices/BundleEvent.h"

namespace cppmicroservices
{
    void BPInstalledState::Start(BundlePrivate& mgr, uint32_t options)
    {
        CheckFrameworkHasStopped(mgr);
        SetAutostart(mgr, options);

        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto resolvedState = std::make_shared<BPResolvedState>(std::move(fut)); 

        while(currState->GetState() == Bundle::STATE_INSTALLED){
            if (mgr.CompareAndSetState(&currState, resolvedState)){
                currState->WaitForTransitionTask();                   
                mgr.coreCtx->listeners.BundleChanged(
                    { BundleEvent::BUNDLE_RESOLVED, MakeBundle(mgr.shared_from_this()) });
                transitionAction.set_value();
                resolvedState->Start(mgr, options); //call resolved->start now
                break;
            }
        }
    }

} 
