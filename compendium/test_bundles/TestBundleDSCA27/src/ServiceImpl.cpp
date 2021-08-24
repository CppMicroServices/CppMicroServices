#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

void ServiceComponentCA27::Modified(
  const std::shared_ptr<ComponentContext>& /*context*/,
  const std::shared_ptr<cppmicroservices::AnyMap>& configuration)
{
  std::lock_guard<std::mutex> lock(propertiesLock);
  properties = configuration;
}

cppmicroservices::AnyMap ServiceComponentCA27::GetProperties()
{
  std::lock_guard<std::mutex> lock(propertiesLock);
  return *properties;
}

}
