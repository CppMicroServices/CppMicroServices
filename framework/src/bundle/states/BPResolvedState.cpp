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
        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto startingState = std::make_shared<BPStartingState>(std::move(fut));

        while(currState->GetState() == Bundle::STATE_RESOLVED){
            if (mgr.CompareAndSetState(&currState, startingState)){
                currState->WaitForTransitionTask();

                auto frameworkBlock = CheckAndBlockFramework(mgr);
                US_UNUSED(frameworkBlock);
                SetAutostart(mgr, options);

                std::shared_ptr<BundleContextPrivate> null_expected;
                std::shared_ptr<BundleContextPrivate> ctx(new BundleContextPrivate(&mgr));
                mgr.bundleContext.CompareExchange(null_expected, ctx);
                auto const thisBundle = MakeBundle(mgr.shared_from_this());
                mgr.coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::BUNDLE_STARTING, thisBundle));
                
                transitionAction.set_value();
                startingState->Start(mgr, options);
                break;
            }
        }
    }

    void BPResolvedState::Stop(BundlePrivate& mgr, uint32_t options){
        SetAutostart(mgr, options);
        return;
    };

    void BPResolvedState::Uninstall(BundlePrivate& mgr){
        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto installedState = std::make_shared<BPInstalledState>(std::move(fut));

        while(currState->GetState() == Bundle::STATE_RESOLVED){
            if (mgr.CompareAndSetState(&currState, installedState))
            {
                currState->WaitForTransitionTask();
                transitionAction.set_value();
                installedState->Uninstall(mgr);
                break;
            }
        }
    };

    
} 
