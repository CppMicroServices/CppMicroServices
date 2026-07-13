#include "BPIdleState.h"
#include "BPResolvedState.h"
#include "BPInstalledState.h"
#include "BPUninstalledState.h"
#include "BundlePrivate.h"
#include "CoreBundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/FrameworkEvent.h"

#include "BundleContextPrivate.h"
#include "cppmicroservices/util/FileSystem.h"
#include <chrono>


namespace cppmicroservices
{
    void BPIdleState::Stop(BundlePrivate& mgr, uint32_t options){
        SetAutostart(mgr, options);
        return;
    };

    void BPIdleState::Uninstall(BundlePrivate& mgr){

        //promise for uninstalled
        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto uninstalledState = std::make_shared<BPUninstalledState>(std::move(fut));

        while(currState->GetState() != Bundle::STATE_UNINSTALLED){
            if (mgr.CompareAndSetState(&currState, uninstalledState))
            {
                currState->WaitForTransitionTask();
                FinalizeUninstall(mgr);
                transitionAction.set_value();
                break;
            }
        }
        
    };
} 
