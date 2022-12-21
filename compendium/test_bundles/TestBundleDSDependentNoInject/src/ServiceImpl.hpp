#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace dependent
{
    class TestBundleDSDependentNoInjectImpl : public test::TestBundleDSDependent
    {
      public:
        TestBundleDSDependentNoInjectImpl();
        ~TestBundleDSDependentNoInjectImpl() override;

        void
        Activate(std::shared_ptr<ComponentContext> const& context)
        {
            ref = context->LocateService<test::TestBundleDSUpstreamDependency>("testbundledsupstreamdependency");
        }

        void
        Deactivate(std::shared_ptr<ComponentContext> const&)
        {
        }

        void
        Bindtestbundledsupstreamdependency(std::shared_ptr<test::TestBundleDSUpstreamDependency> const&)
        {
        }

        void
        Unbindtestbundledsupstreamdependency(std::shared_ptr<test::TestBundleDSUpstreamDependency> const&)
        {
        }

      private:
        std::shared_ptr<test::TestBundleDSUpstreamDependency> ref;
    };
} // namespace dependent

#endif // _SERVICE_IMPL_HPP_
