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

}
