#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample {
class ServiceComponentCA24 : public test::CAInterface
{
public:
  ServiceComponentCA24(const std::shared_ptr<cppmicroservices::AnyMap>& props)
    : properties(props)
  {}
  void Modified(const std::shared_ptr<ComponentContext>& context,
                const std::shared_ptr<cppmicroservices::AnyMap>& configuration);
  cppmicroservices::AnyMap GetProperties();
  ~ServiceComponentCA24() = default;

private:
  std::mutex propertiesLock;
  std::shared_ptr<cppmicroservices::AnyMap> properties;
};
}

#endif // _SERVICE_IMPL_HPP_
