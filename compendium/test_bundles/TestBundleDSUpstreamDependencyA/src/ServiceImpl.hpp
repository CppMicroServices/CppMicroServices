#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

#include "TestInterfaces/Interfaces.hpp"

namespace dependent {
class TestBundleDSUpstreamDependencyImpl
  : public test::TestBundleDSUpstreamDependency
{
public:
  TestBundleDSUpstreamDependencyImpl();
  ~TestBundleDSUpstreamDependencyImpl() override;

  void Activate(const std::shared_ptr<cppmicroservices::service::component::ComponentContext>& context)
  {
    ctx = context->GetBundleContext();  
  }

  void Deactivate(const std::shared_ptr<cppmicroservices::service::component::ComponentContext>&) {}

private:
  cppmicroservices::BundleContext ctx;
};
}

#endif // _SERVICE_IMPL_HPP_
