#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "TestInterfaces/Interfaces.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample {

class ServiceComponent22
  : public test::Interface2
{
public:
  ServiceComponent22() = default;
  std::string ExtendedDescription() override;
  ~ServiceComponent22() = default;

  void Bindfoo(const std::shared_ptr<test::Interface1>&);
  void Unbindfoo(const std::shared_ptr<test::Interface1>&);
private:
  std::shared_ptr<test::Interface1> foo;
};

} // namespaces

#endif // _SERVICE_IMPL_HPP_
