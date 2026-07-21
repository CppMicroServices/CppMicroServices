#include "BPResolvedState.h"
#include "BPStartingState.h"
#include "BPStoppingState.h"
#include "BPInstalledState.h"
#include "BundlePrivate.h"
#include "BundleContextPrivate.h"
#include "CoreBundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/util/Error.h"
#include "cppmicroservices/util/FileSystem.h"
#include "cppmicroservices/util/String.h"
#include "cppmicroservices/SharedLibraryException.h"
#include "BundleUtils.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/AnyMap.h"

namespace cppmicroservices
{
    void BPResolvedState::Start(BundlePrivate& mgr, uint32_t options){
        CheckFrameworkHasStopped(mgr);
        SetAutostart(mgr, options);

        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto startingState = std::make_shared<BPStartingState>(std::move(fut));

        while(currState->GetState() == Bundle::STATE_RESOLVED){
            if (mgr.CompareAndSetState(&currState, startingState)){
                currState->WaitForTransitionTask();
                std::shared_ptr<BundleContextPrivate> null_expected;
                std::shared_ptr<BundleContextPrivate> ctx(new BundleContextPrivate(&mgr));
                mgr.bundleContext.CompareExchange(null_expected, ctx);
                auto const thisBundle = MakeBundle(mgr.shared_from_this());
                mgr.coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::BUNDLE_STARTING, thisBundle));
                transitionAction.set_value();
                startingState->Start(mgr, options);
            }
        }
    }

    void BPResolvedState::Uninstall(BundlePrivate& mgr){

        // auto currState = shared_from_this(); 
        // std::promise<void> transitionAction; 
        // auto fut = transitionAction.get_future();
        // auto installedState = std::make_shared<BPInstalledState>(std::move(fut));

        // while(currState->GetState() != Bundle::STATE_INSTALLED){
        //     if (mgr.CompareAndSetState(&currState, installedState))
        //     {
        //         currState->WaitForTransitionTask();
        //         mgr.coreCtx->listeners.BundleChanged(
        //                 { BundleEvent::BUNDLE_UNRESOLVED, MakeBundle(mgr.shared_from_this()) });
        //         transitionAction.set_value();
        //         installedState->Uninstall(mgr);
        //         break;
        //     }
        // }

        auto currState = shared_from_this(); 
        auto installedState = std::make_shared<BPInstalledState>();

        while(currState->GetState() != Bundle::STATE_INSTALLED){
            if (mgr.CompareAndSetState(&currState, installedState))
            {
                installedState->Uninstall(mgr);
                break;
            }
        }
        
    };

    
} 
