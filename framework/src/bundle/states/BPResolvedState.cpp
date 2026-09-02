#include "BPResolvedState.h"
#include "BPActiveState.h"
#include "BPInstalledState.h"
#include "BundlePrivate.h"
#include "BundleContextPrivate.h"
#include "CoreBundleContext.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/util/Error.h"
#include "cppmicroservices/util/FileSystem.h"
#include "cppmicroservices/util/String.h"
#include "BundleUtils.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/SharedLibraryException.h"

namespace cppmicroservices
{
    void BPResolvedState::Start(BundlePrivate& mgr, uint32_t options){
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
            SetAutostart(mgr, options, options);
            StartFromIdle(mgr, newState);
        }
        else {
            observedState->WaitForTransitionTask();
        }        
    }

    void BPResolvedState::Stop(BundlePrivate& mgr, uint32_t options){
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
        }
        else {
            observedState->WaitForTransitionTask();
        }        
    }

    void BPResolvedState::Uninstall(BundlePrivate& mgr){
        TransitionLogger transitionLogger(mgr, "Uninstall()");
        auto observedState = shared_from_this(); 
        std::promise<void> transitionAction; 
        auto fut = transitionAction.get_future();
        auto newState = std::make_shared<BPInstalledState>(std::move(fut));

        observedState->WaitForTransitionTask();
        if (mgr.CompareAndSetState(&observedState, newState))
        {
            transitionLogger.MarkTransitionAccepted();
            TransitionCompletionGuard completeTransition(transitionAction);
            completeTransition.Complete();
            newState->Uninstall(mgr);
        }
        else {
            observedState->WaitForTransitionTask();
        }        
    }

    
} 
