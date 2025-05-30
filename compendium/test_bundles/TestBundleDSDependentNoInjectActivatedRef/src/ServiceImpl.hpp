#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include <iostream>
#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace dependent
{
    class TestBundleDSDependentNoInjectVerifyRefActive : public test::TestBundleDSDependent
    {
      public:
        TestBundleDSDependentNoInjectVerifyRefActive();
        ~TestBundleDSDependentNoInjectVerifyRefActive() override;

        void
        Activate(std::shared_ptr<ComponentContext> const& context)
        {

            // verify there is only one service created and it is injected here. This means that locateService
            // actually returns the pointer stored in the boundServicesCache
            ref = context->LocateService<test::TestBundleDSUpstreamDependencyIsActivated>("testbundledsupstreamdependencyisactivated");
            if(!ref || !ref->isActivated() || ref->numberCreated() != 1u){
              throw std::runtime_error("Failed to create TestBundleDSUpstreamDependencyIsActivatedImpl");
            }
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
        std::shared_ptr<test::TestBundleDSUpstreamDependencyIsActivated> ref;
    };
} // namespace dependent

#endif // _SERVICE_IMPL_HPP_
