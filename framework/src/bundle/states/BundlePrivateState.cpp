#include "BundlePrivateState.h"
#include "BundlePrivate.h"
#include "CoreBundleContext.h"
#include "cppmicroservices/Bundle.h"
#include <sstream>
#include <exception>

namespace cppmicroservices {

    BundlePrivateState::BundlePrivateState() 
    : ownerThread(std::this_thread::get_id())
    {
        std::promise<void> prom;
        ready = prom.get_future().share();
        prom.set_value();
    }

    BundlePrivateState::BundlePrivateState(std::shared_future<void> blockUntil)
    : ready(std::move(blockUntil))
    , ownerThread(std::this_thread::get_id())
    {
    }

    void BundlePrivateState::SetAutostart(BundlePrivate& mgr, uint32_t options, uint32_t autostartValue){
        if ((options & Bundle::START_TRANSIENT) == 0)
        {
            mgr.SetAutostartSetting(autostartValue);
        }
    }

    std::unique_ptr<FrameworkShutdownBlocker> BundlePrivateState::CheckAndBlockFramework(BundlePrivate& mgr){
        auto frameworkBlock = mgr.coreCtx->GetFrameworkStateAndBlock();
        if (frameworkBlock->frameworkHasStopped)
        {
            throw std::runtime_error("Bundle " + mgr.symbolicName + " (location=" + mgr.location
                                     + ") belongs to a stopped framework");
        }
        return frameworkBlock;
    }

    void BundlePrivateState::WaitForTransitionTask()
    {
        ready.get();
    }

    TransitionCompletionGuard::TransitionCompletionGuard(std::promise<void>& transitionAction)
    : transitionAction(&transitionAction)
    {
    }

    TransitionCompletionGuard::~TransitionCompletionGuard()
    {
        Complete();
    }

    void
    TransitionCompletionGuard::Complete()
    {
        if (transitionAction != nullptr)
        {
            transitionAction->set_value();
            transitionAction = nullptr;
        }
    }

    TransitionLogger::TransitionLogger(BundlePrivate& mgr, std::string transitionName)
        : mgr(mgr)
        , transitionName(std::move(transitionName))
        , successfulTransition(false)
        , uncaughtExceptionCount(std::uncaught_exceptions())
    {
    }

    TransitionLogger::~TransitionLogger()
    {
        if (!successfulTransition && std::uncaught_exceptions() == uncaughtExceptionCount)
        {
            std::ostringstream msg;
            msg << "Dropped bundle lifecycle transition '" << transitionName
                << "' because another transition completed first. This can happen when the same bundle is started, stopped, or uninstalled concurrently."
                << "\nBundle: " << mgr.symbolicName
                << " (location=" << mgr.location << ")\n";

            mgr.coreCtx->logger->Log(logservice::SeverityLevel::LOG_DEBUG, msg.str());
        }
    }

    void
    TransitionLogger::MarkTransitionAccepted()
    {
        successfulTransition = true;
    }
}
