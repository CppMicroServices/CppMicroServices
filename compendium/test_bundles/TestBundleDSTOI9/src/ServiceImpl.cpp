#include "ServiceImpl.hpp"

namespace sample {

void ServiceComponent9::Activate(const std::shared_ptr<ComponentContext>&)
{
  activated = true;
  throw std::runtime_error("Exception thrown from user code");
};
void ServiceComponent9::Deactivate(const std::shared_ptr<ComponentContext>&)
{
  deactivated = true;
  throw std::runtime_error("Exception thrown from user code");
};

} // namespaces
