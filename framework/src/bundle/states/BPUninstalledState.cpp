#include "BPUninstalledState.h"
#include "BundlePrivate.h"
#include "CoreBundleContext.h"
#include "cppmicroservices/Bundle.h"

namespace cppmicroservices
{
    
    uint32_t BPUninstalledState::GetState(){
        return Bundle::STATE_UNINSTALLED;
    };

    void BPUninstalledState::Start(BundlePrivate& mgr, uint32_t options){
        US_UNUSED(options);
        auto frameworkBlock = CheckAndBlockFramework(mgr);
        US_UNUSED(frameworkBlock);
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
