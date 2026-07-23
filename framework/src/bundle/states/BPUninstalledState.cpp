#include "BPUninstalledState.h"
#include "BundlePrivate.h"
#include "CoreBundleContext.h"

namespace cppmicroservices
{

    void BPUninstalledState::Start(BundlePrivate& mgr, uint32_t options){
        US_UNUSED(options);
        CheckFrameworkHasStopped(mgr);
        throw std::logic_error("Bundle " + mgr.symbolicName + " (location=" + mgr.location + ") is uninstalled");
    }

    void BPUninstalledState::Stop(BundlePrivate& mgr, uint32_t options){
        US_UNUSED(options);
        throw std::logic_error("Bundle " + mgr.symbolicName + " (location=" + mgr.location + ") is uninstalled");
    }

    void BPUninstalledState::Uninstall(BundlePrivate& mgr){
        throw std::logic_error("Bundle " + mgr.symbolicName + " (location=" + mgr.location
                        + ") is in BUNDLE_UNINSTALLED state");
    }

} 
