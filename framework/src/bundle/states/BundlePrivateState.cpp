#include "BundlePrivateState.h"
#include "BundlePrivate.h"
#include "CoreBundleContext.h"
#include "cppmicroservices/Bundle.h"

namespace cppmicroservices {

    BundlePrivateState::BundlePrivateState()
    {
        std::promise<void> prom;
        ready = prom.get_future().share();
        prom.set_value();
    }

    BundlePrivateState::BundlePrivateState(std::shared_future<void> blockUntil)
    : ready(std::move(blockUntil))
    {
    }

    void BundlePrivateState::SetAutostart(BundlePrivate& mgr, uint32_t options){
        if ((options & Bundle::START_TRANSIENT) == 0)
        {
            mgr.SetAutostartSetting(options);
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

    std::string BundleStateToString(uint32_t state)
    {
        switch (state)
        {
            case Bundle::STATE_UNINSTALLED:
                return "STATE_UNINSTALLED";
            case Bundle::STATE_INSTALLED:
                return "STATE_INSTALLED";
            case Bundle::STATE_RESOLVED:
                return "STATE_RESOLVED";
            case Bundle::STATE_STARTING:
                return "STATE_STARTING";
            case Bundle::STATE_STOPPING:
                return "STATE_STOPPING";
            case Bundle::STATE_ACTIVE:
                return "STATE_ACTIVE";
            default:
                return "UNKNOWN_STATE(" + std::to_string(state) + ")";
        }
    }

    void BundlePrivateState::LogDroppedTransition(BundlePrivate& mgr,
                                                std::string const& transitionName,
                                                uint32_t expectedState,
                                                uint32_t actualState)
    {
        mgr.coreCtx->logger->Log(
            logservice::SeverityLevel::LOG_DEBUG,
            "Dropped bundle lifecycle transition '" + transitionName
                + "' for Bundle " + mgr.symbolicName
                + " (location=" + mgr.location + ")"
                + "; expected state=" + BundleStateToString(expectedState)
                + ", actual state=" + BundleStateToString(actualState));
    }
}
