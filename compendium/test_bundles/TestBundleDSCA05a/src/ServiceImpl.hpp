#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample {
class ServiceComponentCA05a : public test::CAInterface
{
public:
  ServiceComponentCA05a(const std::shared_ptr<cppmicroservices::AnyMap>& props)
    : properties(props)
  {}

  cppmicroservices::AnyMap GetProperties();

  ~ServiceComponentCA05a() = default;

private:
  std::mutex propertiesLock;
  std::shared_ptr<cppmicroservices::AnyMap> properties;
};
}

#endif // _SERVICE_IMPL_HPP_
