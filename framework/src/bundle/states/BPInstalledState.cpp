#include "BPInstalledState.h"
#include "BPUninstalledState.h"
#include "BPActiveState.h"
#include "CoreBundleContext.h"
#include "BundlePrivate.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/util/FileSystem.h"
#include <chrono>

namespace cppmicroservices
{

    uint32_t BPInstalledState::GetState(){
        return Bundle::STATE_INSTALLED;
    }

    void BPInstalledState::Start(BundlePrivate& mgr, uint32_t options)
    {
        TransitionLogger transitionLogger(mgr, "Start()", Bundle::STATE_INSTALLED);
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto newState = std::make_shared<BPActiveState>(std::move(fut)); 

        observedState->WaitForTransitionTask();
        if (mgr.CompareAndSetState(&observedState, newState)){
            transitionLogger.MarkTransitionAccepted();
            TransitionCompletionGuard completeTransition(transitionAction);

            auto frameworkBlock = CheckAndBlockFramework(mgr);
            SetAutostart(mgr, options, options);
            mgr.SetStateValue(Bundle::STATE_RESOLVED);
            mgr.coreCtx->listeners.BundleChanged(
                { BundleEvent::BUNDLE_RESOLVED, MakeBundle(mgr.shared_from_this()) });
            StartFromIdle(mgr, newState);
        }

        transitionLogger.SetActualState(observedState);
    }

    void BPInstalledState::Stop(BundlePrivate& mgr, uint32_t options){
        TransitionLogger transitionLogger(mgr, "Stop()", Bundle::STATE_INSTALLED);
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto newState = std::make_shared<BPInstalledState>(std::move(fut)); 

        observedState->WaitForTransitionTask();
        if (mgr.CompareAndSetState(&observedState, newState)){
            transitionLogger.MarkTransitionAccepted();
            TransitionCompletionGuard completeTransition(transitionAction);
            SetAutostart(mgr, options, -1);
        }

        transitionLogger.SetActualState(observedState);
    }

    void BPInstalledState::Uninstall(BundlePrivate& mgr){

        TransitionLogger transitionLogger(mgr, "Uninstall()", Bundle::STATE_INSTALLED);
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto newState = std::make_shared<BPUninstalledState>(std::move(fut));

        observedState->WaitForTransitionTask();
        if (mgr.CompareAndSetState(&observedState, newState))
        {
            transitionLogger.MarkTransitionAccepted();
            TransitionCompletionGuard completeTransition(transitionAction);
            mgr.coreCtx->bundleRegistry.Remove(mgr.location, mgr.id);
            mgr.SetStateValue(Bundle::STATE_INSTALLED);
            mgr.coreCtx->listeners.BundleChanged(
                { BundleEvent::BUNDLE_UNRESOLVED, MakeBundle(mgr.shared_from_this()) });
            mgr.bactivator = nullptr;
            mgr.Purge();
            mgr.barchive->SetLastModified(std::chrono::steady_clock::now());
            if (!mgr.bundleDir.empty())
            {
                try
                {
                    if (util::Exists(mgr.bundleDir))
                    {
                        util::RemoveDirectoryRecursive(mgr.bundleDir);
                    }
                }
                catch (...)
                {
                    mgr.coreCtx->listeners.SendFrameworkEvent(
                        FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_WARNING,
                                        MakeBundle(mgr.shared_from_this()),
                                        std::string(),
                                        std::current_exception()));
                }
                mgr.bundleDir.clear();
            }
            mgr.SetStateValue(Bundle::STATE_UNINSTALLED);
            mgr.coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::BUNDLE_UNINSTALLED, MakeBundle(mgr.shared_from_this())));
        }

        transitionLogger.SetActualState(observedState);
        
    }

} 
