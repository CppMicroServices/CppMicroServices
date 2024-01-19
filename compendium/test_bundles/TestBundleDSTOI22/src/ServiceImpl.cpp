#include "ServiceImpl.hpp"
#include "cppmicroservices/GetBundleContext.h"

namespace sample
{

    void
    ServiceComponent22::Activate(std::shared_ptr<ComponentContext> const& ctx)
    {
        providedCtx = ctx->GetBundleContext();
    }

    void
    ServiceComponent22::Deactivate(std::shared_ptr<ComponentContext> const&)
    {
    }

    std::vector<cppmicroservices::BundleContext>
    ServiceComponent22::GetContexts(void)
    {
        BundleContext retrievedCtx = cppmicroservices::GetBundleContext();
        return std::vector<BundleContext>{providedCtx, retrievedCtx, activatorProvidedCtx};
    }

} // namespace sample
