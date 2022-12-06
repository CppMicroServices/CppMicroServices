#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace dependent {
class TestBundleDSDependentImpl : public test::TestBundleDSDependent
{
public:
  TestBundleDSDependentImpl(
    const std::shared_ptr<test::TestBundleDSUpstreamDependency>&);
  ~TestBundleDSDependentImpl() override;

  void Activate(const std::shared_ptr<ComponentContext>&) {}

  void Deactivate(const std::shared_ptr<ComponentContext>&) {}

private:
  std::shared_ptr<test::TestBundleDSUpstreamDependency> ref;
};
}

#endif // _SERVICE_IMPL_HPP_
