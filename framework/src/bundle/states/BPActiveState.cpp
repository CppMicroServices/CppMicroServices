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
    
    void BPActiveState::Start(BundlePrivate& mgr, uint32_t options){
        CheckFrameworkHasStopped(mgr);
        US_UNUSED(options);
        return;
    };

    void BPActiveState::Stop(BundlePrivate& mgr, uint32_t options){
        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto stoppingState = std::make_shared<BPStoppingState>(std::move(fut));

        while(currState->GetState() != Bundle::STATE_STOPPING){
            if (mgr.CompareAndSetState(&currState, stoppingState)){
                currState->WaitForTransitionTask();
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

                std::shared_ptr<BundleContextPrivate> ctx = mgr.bundleContext.Load();
                if (ctx)
                {
                    mgr.coreCtx->listeners.HooksBundleStopped(ctx);
                    mgr.RemoveBundleResources();
                    ctx->Invalidate();
                    mgr.bundleContext.Store(std::shared_ptr<BundleContextPrivate>());
                }

                transitionAction.set_value();
                stoppingState->Stop(mgr, options);
                if (res){
                    std::rethrow_exception(res);
                }
                break;
            }
        }
    }

    void BPActiveState::Uninstall(BundlePrivate& mgr){
        auto currState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto stoppingState = std::make_shared<BPStoppingState>(std::move(fut));

        while(currState->GetState() != Bundle::STATE_STOPPING){
            if (mgr.CompareAndSetState(&currState, stoppingState)){
                currState->WaitForTransitionTask();
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

                std::shared_ptr<BundleContextPrivate> ctx = mgr.bundleContext.Load();
                if (ctx)
                {
                    mgr.coreCtx->listeners.HooksBundleStopped(ctx);
                    mgr.RemoveBundleResources();
                    ctx->Invalidate();
                    mgr.bundleContext.Store(std::shared_ptr<BundleContextPrivate>());
                }

                if (res){
                    mgr.coreCtx->listeners.SendFrameworkEvent(FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_ERROR,
                                                                                MakeBundle(mgr.shared_from_this()),
                                                                                std::string(),
                                                                                res));
                }
                
                transitionAction.set_value();
                stoppingState->Uninstall(mgr);
                break;
            }
        }
    }

} 
