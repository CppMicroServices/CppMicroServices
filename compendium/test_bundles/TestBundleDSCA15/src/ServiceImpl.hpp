#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample {
class ServiceComponentCA15 : public test::CAInterface
{
public:
  ServiceComponentCA15() = default;

  void Modified(const std::shared_ptr<ComponentContext>& context,
                const std::shared_ptr<cppmicroservices::AnyMap>& configuration);
  cppmicroservices::AnyMap GetProperties();

  ~ServiceComponentCA15() = default;

private:
  std::mutex propertiesLock;
  cppmicroservices::AnyMap properties{
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS
  };
};
}

#endif // _SERVICE_IMPL_HPP_
