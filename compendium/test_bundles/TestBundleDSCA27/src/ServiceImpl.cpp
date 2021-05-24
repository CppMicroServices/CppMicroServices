#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

cppmicroservices::AnyMap ServiceComponentCA27::GetProperties()
{
  std::lock_guard<std::mutex> lock(propertiesLock);
  return properties;
}

}
