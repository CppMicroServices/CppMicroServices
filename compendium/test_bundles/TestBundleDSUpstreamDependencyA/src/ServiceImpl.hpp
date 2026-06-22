#ifndef SERVICE_IMPL_DSUPSTREAMDEPENDENCYA_HPP
#define SERVICE_IMPL_DSUPSTREAMDEPENDENCYA_HPP

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

#include "TestInterfaces/Interfaces.hpp"

namespace dependent
{
    class TestBundleDSUpstreamDependencyImpl : public test::TestBundleDSUpstreamDependency
    {
      public:
        TestBundleDSUpstreamDependencyImpl();
        ~TestBundleDSUpstreamDependencyImpl() override;

        void
        Activate(std::shared_ptr<cppmicroservices::service::component::ComponentContext> const& context)
        {
            ctx = context->GetBundleContext();
        }

        void
        Deactivate(std::shared_ptr<cppmicroservices::service::component::ComponentContext> const&)
        {
        }

      private:
        cppmicroservices::BundleContext ctx;
    };
} // namespace dependent

#endif // SERVICE_IMPL_DSUPSTREAMDEPENDENCYA_HPP
