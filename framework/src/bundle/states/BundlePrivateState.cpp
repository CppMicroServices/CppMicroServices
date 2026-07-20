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

    void BundlePrivateState::StartFailed(BundlePrivate& mgr, std::shared_ptr<BundlePrivateState> expectedState)
    {
        // auto stoppingState = std::make_shared<BPStoppingState>();
        // mgr.CompareAndSetState(&expectedState, stoppingState);
        mgr.coreCtx->listeners.BundleChanged(
            BundleEvent(BundleEvent::BUNDLE_STOPPING, MakeBundle(mgr.shared_from_this())));
        mgr.RemoveBundleResources();
        auto oldBundleContext = mgr.bundleContext.Exchange(std::shared_ptr<BundleContextPrivate>());
        if (oldBundleContext)
        {
            oldBundleContext->Invalidate();
        }

        mgr.CompareAndSetState(&expectedState, std::make_shared<BPResolvedState>());

        mgr.coreCtx->listeners.BundleChanged(
            BundleEvent(BundleEvent::BUNDLE_STOPPED, MakeBundle(mgr.shared_from_this())));
        

    }
}
