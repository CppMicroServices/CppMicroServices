#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace dependent {
class TestBundleDSDependentNoInjectImpl : public test::TestBundleDSDependent
{
public:
  TestBundleDSDependentNoInjectImpl();
  ~TestBundleDSDependentNoInjectImpl() override;

  void Activate(const std::shared_ptr<ComponentContext>& context)
  {
    ref = context->LocateService<test::TestBundleDSUpstreamDependency>(
      "testbundledsupstreamdependency");
  }

  void Deactivate(const std::shared_ptr<ComponentContext>& context) {}

  void Bindtestbundledsupstreamdependency(
    const std::shared_ptr<test::TestBundleDSUpstreamDependency>& obj)
  {}
  void Unbindtestbundledsupstreamdependency(
    const std::shared_ptr<test::TestBundleDSUpstreamDependency>& obj)
  {}

private:
  std::shared_ptr<test::TestBundleDSUpstreamDependency> ref;
};
}

#endif // _SERVICE_IMPL_HPP_
