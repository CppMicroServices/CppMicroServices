#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace dependent
{
    class TestBundleDSDependentImpl : public test::TestBundleDSDependent
    {
      public:
        TestBundleDSDependentImpl(std::shared_ptr<test::TestBundleDSUpstreamDependency> const&);
        ~TestBundleDSDependentImpl() override;

        void
        Activate(std::shared_ptr<ComponentContext> const&)
        {
        }

        void
        Deactivate(std::shared_ptr<ComponentContext> const&)
        {
        }

      private:
        std::shared_ptr<test::TestBundleDSUpstreamDependency> ref;
    };
} // namespace dependent

#endif // _SERVICE_IMPL_HPP_
