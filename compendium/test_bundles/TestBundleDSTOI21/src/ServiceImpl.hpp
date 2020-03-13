#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "TestInterfaces/Interfaces.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample {

class ServiceComponent21
  : public test::Interface1
{
public:
  ServiceComponent21() = default;
  std::string Description() override;
  ~ServiceComponent21() = default;
};

} // namespaces

#endif // _SERVICE_IMPL_HPP_
