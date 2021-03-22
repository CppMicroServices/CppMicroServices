#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

void TestBundleDSCAI1::Modified(
  const std::shared_ptr<ComponentContext>& context,
  const std::shared_ptr<cppmicroservices::AnyMap>& configuration)
{
  std::lock_guard<std::mutex> lock(propertiesLock);
  properties = *configuration;
}

}
