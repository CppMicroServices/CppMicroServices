#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample {
class ServiceComponent20 : public test::Interface2
{
public:
  ServiceComponent20() = default;
  std::string ExtendedDescription() override;
  ~ServiceComponent20() = default;

  void Bindfoo(const std::shared_ptr<test::Interface1>&);
  void Unbindfoo(const std::shared_ptr<test::Interface1>&);

private:
  std::shared_ptr<test::Interface1> foo;
};
}

#endif // _SERVICE_IMPL_HPP_
