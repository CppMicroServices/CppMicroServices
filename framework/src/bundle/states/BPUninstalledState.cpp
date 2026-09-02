#include "BPUninstalledState.h"
#include "BundlePrivate.h"
#include "CoreBundleContext.h"
#include "cppmicroservices/Bundle.h"

namespace cppmicroservices
{
    void BPUninstalledState::Start(BundlePrivate& mgr, uint32_t){
        shared_from_this()->WaitForTransitionTask();
        throw std::logic_error("Bundle " + mgr.symbolicName + " (location=" + mgr.location + ") is uninstalled");
    }

    void BPUninstalledState::Stop(BundlePrivate& mgr, uint32_t){
        shared_from_this()->WaitForTransitionTask();
        throw std::logic_error("Bundle " + mgr.symbolicName + " (location=" + mgr.location + ") is uninstalled");
    }

    void BPUninstalledState::Uninstall(BundlePrivate& mgr){
        shared_from_this()->WaitForTransitionTask();
        throw std::logic_error("Bundle " + mgr.symbolicName + " (location=" + mgr.location
                        + ") is in BUNDLE_UNINSTALLED state");
    }

} 
