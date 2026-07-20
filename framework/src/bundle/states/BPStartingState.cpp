#include "BPStartingState.h"
#include "BPActiveState.h"
#include "BundlePrivate.h"
#include "CoreBundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/SharedLibraryException.h"

namespace cppmicroservices
{

    void BPStartingState::Start(BundlePrivate& mgr, uint32_t options){
        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto activeState = std::make_shared<BPActiveState>(std::move(fut)); 

        while(currState->GetState() == Bundle::STATE_STARTING){
            if (mgr.CompareAndSetState(&currState, activeState)){
                currState->WaitForTransitionTask(); 
                CheckFrameworkHasStopped(mgr);
                SetAutostart(mgr, options);
                mgr.wasStarted = 1; 
                std::exception_ptr res;

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
                    mgr.wasStarted = 0; 
                    transitionAction.set_value();
                    activeState->Stop(mgr, options);
                    throw ex;
                }

                transitionAction.set_value();
                break;
            }
        }
    }

} 
