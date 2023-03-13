#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

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

#endif // _SERVICE_IMPL_HPP_
