#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "TestInterfaces/Interfaces.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample {

class ServiceComponent8
  : public test::Interface1
{
public:
  ServiceComponent8() = default;
  std::string Description() override;
  ~ServiceComponent8() = default;
};

} // namespaces

#endif // _SERVICE_IMPL_HPP_
