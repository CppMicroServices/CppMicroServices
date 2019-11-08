#include "ServiceImpl.hpp"

#include <iostream>
#include <exception>

namespace sample {

ServiceComponent9::ServiceComponent9()
  : test::LifeCycleValidation()
{
  throw std::exception {};
}

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
ServiceComponent9::~ServiceComponent9() {};

} // namespaces
