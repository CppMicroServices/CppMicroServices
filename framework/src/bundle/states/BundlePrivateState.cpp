#include "BundlePrivateState.h"
#include "BundlePrivate.h"
#include "cppmicroservices/Bundle.h"
#include "CoreBundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/util/Error.h"
#include "cppmicroservices/util/FileSystem.h"
#include "cppmicroservices/util/String.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "BundleContextPrivate.h"
#include "BPResolvedState.h"

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

    void BundlePrivateState::CheckFrameworkHasStopped(BundlePrivate& mgr){
        auto frameworkBlock = mgr.coreCtx->GetFrameworkStateAndBlock();
        if (frameworkBlock->frameworkHasStopped)
        {
            throw std::runtime_error("Bundle " + mgr.symbolicName + " (location=" + mgr.location
                                     + ") belongs to a stopped framework");
        }
    }

    void BundlePrivateState::WaitForTransitionTask()
    {
        ready.get();
    }

}
