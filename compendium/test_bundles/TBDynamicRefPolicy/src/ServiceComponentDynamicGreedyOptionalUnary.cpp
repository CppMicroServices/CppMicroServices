#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

void ServiceComponentDynamicGreedyOptionalUnary::Activate(const std::shared_ptr<ComponentContext>& /*ctxt*/)
{
}
  
void ServiceComponentDynamicGreedyOptionalUnary::Deactivate(const std::shared_ptr<ComponentContext>&)
{
}

std::string ServiceComponentDynamicGreedyOptionalUnary::ExtendedDescription()
{
  if(!foo)
  {
    throw std::runtime_error("Dependency not available");
  }
  std::string result("ServiceComponentDynamicGreedyOptionalUnary ");
  result.append("depends on ");
  result.append(foo->Description());
  return result;
}

void ServiceComponentDynamicGreedyOptionalUnary::Bindfoo(const std::shared_ptr<test::Interface6>& theFoo)
{
  if (foo != theFoo)
  {
    foo = theFoo;
  }
}

void ServiceComponentDynamicGreedyOptionalUnary::Unbindfoo(const std::shared_ptr<test::Interface6>& theFoo)
{
  if (foo == theFoo)
  {
    foo = nullptr;
  }
}

} // namespaces
