#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

cppmicroservices::AnyMap ServiceComponentCA17::GetProperties()
{
  std::lock_guard<std::mutex> lock(propertiesLock);
  return properties;
}

}
