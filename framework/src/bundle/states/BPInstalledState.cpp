#include "BPInstalledState.h"
#include "BPUninstalledState.h"
#include "BPResolvedState.h"
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
    };

    void BPInstalledState::Start(BundlePrivate& mgr, uint32_t options)
    {
        TransitionLogger transitionLogger(mgr, "Start()", Bundle::STATE_INSTALLED);
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto resolvedState = std::make_shared<BPResolvedState>(std::move(fut)); 

        while(observedState->GetState() == Bundle::STATE_INSTALLED){
            if (mgr.CompareAndSetState(&observedState, resolvedState)){
                observedState->WaitForTransitionTask();      
                
                auto frameworkBlock = CheckAndBlockFramework(mgr);
                SetAutostart(mgr, options);

                mgr.coreCtx->listeners.BundleChanged(
                    { BundleEvent::BUNDLE_RESOLVED, MakeBundle(mgr.shared_from_this()) });
                
                frameworkBlock.reset();
                transitionAction.set_value();
                transitionLogger.TransitionSucceeded();
                resolvedState->Start(mgr, options);
                break;
            }
        }

        transitionLogger.SetActualState(observedState);
    }

    void BPInstalledState::Stop(BundlePrivate& mgr, uint32_t options){
        TransitionLogger transitionLogger(mgr, "Stop()", Bundle::STATE_INSTALLED);
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto installedState = std::make_shared<BPInstalledState>(std::move(fut)); 

        while(observedState->GetState() == Bundle::STATE_INSTALLED){
            if (mgr.CompareAndSetState(&observedState, installedState)){
                observedState->WaitForTransitionTask(); 
                SetAutostart(mgr, options);
                transitionAction.set_value();
                transitionLogger.TransitionSucceeded();
                break;
            }
        }

        transitionLogger.SetActualState(observedState);
    }

    void BPInstalledState::Uninstall(BundlePrivate& mgr){

        TransitionLogger transitionLogger(mgr, "Uninstall()", Bundle::STATE_INSTALLED);
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto uninstalledState = std::make_shared<BPUninstalledState>(std::move(fut));

        while(observedState->GetState() == Bundle::STATE_INSTALLED){
            if (mgr.CompareAndSetState(&observedState, uninstalledState))
            {
                observedState->WaitForTransitionTask();
                mgr.coreCtx->bundleRegistry.Remove(mgr.location, mgr.id);
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
                mgr.coreCtx->listeners.BundleChanged(BundleEvent(BundleEvent::BUNDLE_UNINSTALLED, MakeBundle(mgr.shared_from_this())));

                transitionAction.set_value();
                transitionLogger.TransitionSucceeded();
                break;
            }
        }

        transitionLogger.SetActualState(observedState);
        
    }

} 
