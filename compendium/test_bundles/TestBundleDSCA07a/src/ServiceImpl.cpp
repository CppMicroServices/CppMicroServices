#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

void ServiceComponentCA07a::Modified(
  const std::shared_ptr<ComponentContext>&,
  const std::shared_ptr<cppmicroservices::AnyMap>& configuration)
{
  std::lock_guard<std::mutex> lock(propertiesLock);
  properties = *configuration;
}
cppmicroservices::AnyMap ServiceComponentCA07a::GetProperties()
{
  std::lock_guard<std::mutex> lock(propertiesLock);
  return properties;
}

}
