#include "BPStoppingState.h"
#include "BPActiveState.h"
#include "BundlePrivate.h"
#include "cppmicroservices/BundleActivator.h"
#include "CoreBundleContext.h"
#include "BundleContextPrivate.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/util/Error.h"
#include "cppmicroservices/FrameworkEvent.h"

namespace cppmicroservices
{
    uint32_t BPActiveState::GetState(){
        return Bundle::STATE_ACTIVE;
    };
    
    void BPActiveState::Start(BundlePrivate& mgr, uint32_t options){
        auto frameworkBlock = CheckAndBlockFramework(mgr);
        US_UNUSED(frameworkBlock);
        US_UNUSED(options);
        return;
    };

    namespace {
        std::exception_ptr StopActiveBundle(BundlePrivate& mgr){
            std::exception_ptr res;
            mgr.coreCtx->listeners.BundleChanged(
                BundleEvent(BundleEvent::BUNDLE_STOPPING, MakeBundle(mgr.shared_from_this())));

            if (mgr.bactivator != nullptr)
            {
                try
                {
                    mgr.bactivator->Stop(MakeBundleContext(mgr.bundleContext.Load()));
                }
                catch (...)
                {
                    res = std::make_exception_ptr(
                        std::runtime_error("Bundle " + mgr.symbolicName + " (location=" + mgr.location
                                        + "), BundleActivator::Stop() failed: " + util::GetLastExceptionStr()));
                }
                mgr.bactivator = nullptr;
            }

            if (res){
                mgr.coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_ERROR,
                                                                            MakeBundle(mgr.shared_from_this()),
                                                                            std::string(),
                                                                            res));
            }

            return res;
        }
    }

    void BPActiveState::Stop(BundlePrivate& mgr, uint32_t options){
        TransitionLogger transitionLogger(mgr, "Stop()", Bundle::STATE_ACTIVE);
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto stoppingState = std::make_shared<BPStoppingState>(std::move(fut));

        while(observedState->GetState() == Bundle::STATE_ACTIVE){
            if (mgr.CompareAndSetState(&observedState, stoppingState)){
                TransitionCompletionGuard completeTransition(transitionAction);
                observedState->WaitForTransitionTask();
                SetAutostart(mgr, options);
                std::exception_ptr res = StopActiveBundle(mgr);
                completeTransition.Complete();
                stoppingState->Stop(mgr, options);
                if (res){
                    std::rethrow_exception(res);
                }
                transitionLogger.TransitionSucceeded();
                break;
            }
        }

        transitionLogger.SetActualState(observedState);
    }

    void BPActiveState::Uninstall(BundlePrivate& mgr){
        TransitionLogger transitionLogger(mgr, "Uninstall()", Bundle::STATE_ACTIVE);
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto stoppingState = std::make_shared<BPStoppingState>(std::move(fut));

        while(observedState->GetState() == Bundle::STATE_ACTIVE){
            if (mgr.CompareAndSetState(&observedState, stoppingState)){
                TransitionCompletionGuard completeTransition(transitionAction);
                observedState->WaitForTransitionTask();
                StopActiveBundle(mgr);
                completeTransition.Complete();
                stoppingState->Uninstall(mgr);
                transitionLogger.TransitionSucceeded();
                break;
            }
        }

        transitionLogger.SetActualState(observedState);
    }

} 
