#ifndef SERVICE_IMPL_DSUPSTREAMDEPENDENCYB_HPP
#define SERVICE_IMPL_DSUPSTREAMDEPENDENCYB_HPP

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

namespace dependent
{

    using ComponentContext = cppmicroservices::service::component::ComponentContext;

    class TestBundleDSUpstreamDependencyImpl : public test::TestBundleDSUpstreamDependency
    {
      public:
        TestBundleDSUpstreamDependencyImpl();
        ~TestBundleDSUpstreamDependencyImpl() override;

        void
        Activate(std::shared_ptr<ComponentContext> const&)
        {
            throw std::runtime_error("Failed to create TestBundleDSUpstreamDepdencyImpl");
        }
    };
} // namespace dependent

#endif // SERVICE_IMPL_DSUPSTREAMDEPENDENCYB_HPP
