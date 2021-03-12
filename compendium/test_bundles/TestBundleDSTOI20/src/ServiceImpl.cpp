#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

void ServiceComponent20::Activate(const std::shared_ptr<ComponentContext>& /*ctxt*/)
{
}
  
void ServiceComponent20::Deactivate(const std::shared_ptr<ComponentContext>&)
{
}
void ServiceComponent20::Modified(
  const std::shared_ptr<ComponentContext>& context,
    const std::shared_ptr<cppmicroservices::AnyMap>& configuration)
{
  std::lock_guard<std::mutex> lock(propertiesLock);
  properties = *configuration;
}


}
