#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

namespace dependent {

using ComponentContext = cppmicroservices::service::component::ComponentContext;

class TestBundleDSUpstreamDependencyImpl
  : public test::TestBundleDSUpstreamDependency
{
public:
  TestBundleDSUpstreamDependencyImpl();
  ~TestBundleDSUpstreamDependencyImpl() override;

  void Activate(const std::shared_ptr<ComponentContext>&)
  {
    throw std::runtime_error(
      "Failed to create TestBundleDSUpstreamDepdencyImpl");
  }
};
}

#endif // _SERVICE_IMPL_HPP_
