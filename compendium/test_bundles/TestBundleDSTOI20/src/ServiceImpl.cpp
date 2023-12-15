#include "ServiceImpl.hpp"
#include "cppmicroservices/GetBundleContext.h"

namespace sample
{

    void
    ServiceComponent20::Activate(std::shared_ptr<ComponentContext> const& ctx)
    {
        providedCtx = ctx->GetBundleContext();
    }

    void
    ServiceComponent20::Deactivate(std::shared_ptr<ComponentContext> const&)
    {
    }

    std::vector<cppmicroservices::BundleContext>
    ServiceComponent20::GetContexts(void)
    {
        BundleContext retrievedCtx = cppmicroservices::GetBundleContext();
        return std::vector<BundleContext>{providedCtx, retrievedCtx};
    }

} // namespace sample
