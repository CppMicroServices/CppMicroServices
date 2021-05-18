#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

ServiceComponentCA09::ServiceComponentCA09(const std::shared_ptr<cppmicroservices::AnyMap>& props,
                                           const std::shared_ptr<test::Interface1> interface1)
: properties(*props), constructorHit{false}
{
  if (nullptr != interface1) {
    constructorHit = true;
  }
}
void ServiceComponentCA09::Modified(
  const std::shared_ptr<ComponentContext>& /*context*/,
  const std::shared_ptr<cppmicroservices::AnyMap>& configuration)
{
  std::lock_guard<std::mutex> lock(propertiesLock);
  properties = *configuration;
}
cppmicroservices::AnyMap ServiceComponentCA09::GetProperties()
{
  std::lock_guard<std::mutex> lock(propertiesLock);
  return properties;
}
bool ServiceComponentCA09::isDependencyInjected()
{
  return constructorHit;
}

}
