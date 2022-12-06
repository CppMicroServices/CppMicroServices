#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample {
class ServiceComponentCA16 : public test::CAInterface
{
public:
  ServiceComponentCA16(const std::shared_ptr<cppmicroservices::AnyMap>& props)
    : properties(props)
  {}

  cppmicroservices::AnyMap GetProperties();

  ~ServiceComponentCA16() = default;

private:
  std::mutex propertiesLock;
  std::shared_ptr<cppmicroservices::AnyMap> properties;
};
}

#endif // _SERVICE_IMPL_HPP_
