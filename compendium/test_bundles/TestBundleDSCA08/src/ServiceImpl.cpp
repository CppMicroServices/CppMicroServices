#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

ServiceComponentCA08::ServiceComponentCA08(
  const std::shared_ptr<cppmicroservices::AnyMap>& props)
  : properties(props)
{
}
void ServiceComponentCA08::Modified(
  const std::shared_ptr<ComponentContext>& /*context*/,
  const std::shared_ptr<cppmicroservices::AnyMap>& configuration)
{
  std::lock_guard<std::mutex> lock(propertiesLock);
  properties = configuration;
}
cppmicroservices::AnyMap ServiceComponentCA08::GetProperties()
{
  std::lock_guard<std::mutex> lock(propertiesLock);
  return *properties;
}

}
