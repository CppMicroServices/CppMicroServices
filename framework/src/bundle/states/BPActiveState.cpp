#include "BPActiveState.h"
#include "BPResolvedState.h"
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
    void BPActiveState::Start(BundlePrivate& mgr, uint32_t){
        TransitionLogger transitionLogger(mgr, "Start()");
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto newState = std::make_shared<BPActiveState>(std::move(fut));

        observedState->WaitForTransitionTask();
        if (mgr.CompareAndSetState(&observedState, newState)){
            transitionLogger.MarkTransitionAccepted();
            TransitionCompletionGuard completeTransition(transitionAction);
            auto frameworkBlock = CheckAndBlockFramework(mgr);
            US_UNUSED(frameworkBlock);
        }
        else {
            observedState->WaitForTransitionTask();
        }
    }

    namespace {
        std::exception_ptr StopActiveBundle(BundlePrivate& mgr){
            std::exception_ptr res;
            mgr.SetStateValue(Bundle::STATE_STOPPING);
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

            std::shared_ptr<BundleContextPrivate> ctx = mgr.bundleContext.Load();
            if (ctx)
            {
                mgr.coreCtx->listeners.HooksBundleStopped(ctx);
                mgr.RemoveBundleResources();
                ctx->Invalidate();
                mgr.bundleContext.Store(std::shared_ptr<BundleContextPrivate>());
            }
            mgr.SetStateValue(Bundle::STATE_RESOLVED);
            mgr.coreCtx->listeners.BundleChanged({ BundleEvent::BUNDLE_STOPPED, MakeBundle(mgr.shared_from_this()) }); 

            return res;
        }
    }

    void BPActiveState::Stop(BundlePrivate& mgr, uint32_t options){
        TransitionLogger transitionLogger(mgr, "Stop()");
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto newState = std::make_shared<BPResolvedState>(std::move(fut));

        observedState->WaitForTransitionTask();
        if (mgr.CompareAndSetState(&observedState, newState)){
            transitionLogger.MarkTransitionAccepted();
            TransitionCompletionGuard completeTransition(transitionAction);
            SetAutostart(mgr, options, -1);
            std::exception_ptr res = StopActiveBundle(mgr);
            if (res){
                std::rethrow_exception(res);
            }
        }
        else {
            observedState->WaitForTransitionTask();
        }
    }

    void BPActiveState::Uninstall(BundlePrivate& mgr){
        TransitionLogger transitionLogger(mgr, "Uninstall()");
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto newState = std::make_shared<BPResolvedState>(std::move(fut));

        observedState->WaitForTransitionTask();
        if (mgr.CompareAndSetState(&observedState, newState)){
            transitionLogger.MarkTransitionAccepted();
            TransitionCompletionGuard completeTransition(transitionAction);
            std::exception_ptr res = StopActiveBundle(mgr);
            if (res){
                mgr.coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_ERROR,
                                                                            MakeBundle(mgr.shared_from_this()),
                                                                            std::string(),
                                                                            res));
            }
            completeTransition.Complete();
            newState->Uninstall(mgr);
        }
        else {
            observedState->WaitForTransitionTask();
        }        
    }

    void BPActiveState::StartFailed(BundlePrivate& mgr){
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto newState = std::make_shared<BPResolvedState>(std::move(fut));
        if(mgr.CompareAndSetState(&observedState, newState)){
            TransitionCompletionGuard completeTransition(transitionAction);
            mgr.SetStateValue(Bundle::STATE_STOPPING);
            mgr.coreCtx->listeners.BundleChanged(
                BundleEvent(BundleEvent::BUNDLE_STOPPING, MakeBundle(mgr.shared_from_this())));
            mgr.RemoveBundleResources();
            auto oldBundleContext = mgr.bundleContext.Exchange(std::shared_ptr<BundleContextPrivate>());
            if (oldBundleContext)
            {
                oldBundleContext->Invalidate();
            }
            mgr.SetStateValue(Bundle::STATE_RESOLVED);
            mgr.coreCtx->listeners.BundleChanged({ BundleEvent::BUNDLE_STOPPED, MakeBundle(mgr.shared_from_this()) }); 
        }
        
    }

} 
