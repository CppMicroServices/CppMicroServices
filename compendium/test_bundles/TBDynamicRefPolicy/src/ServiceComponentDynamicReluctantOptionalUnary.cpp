#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

void ServiceComponentDynamicReluctantOptionalUnary::Activate(const std::shared_ptr<ComponentContext>& /*ctxt*/)
{
}
  
void ServiceComponentDynamicReluctantOptionalUnary::Deactivate(const std::shared_ptr<ComponentContext>&)
{
}

std::string ServiceComponentDynamicReluctantOptionalUnary::ExtendedDescription()
{
  if(!foo)
  {
    throw std::runtime_error("Dependency not available");
  }
  std::string result("ServiceComponentDynamicReluctantOptionalUnary ");
  result.append("depends on ");
  result.append(foo->Description());
  return result;
}

void ServiceComponentDynamicReluctantOptionalUnary::Bindfoo(const std::shared_ptr<test::Interface5>& theFoo)
{
  if (foo != theFoo)
  {
    foo = theFoo;
  }
}

void ServiceComponentDynamicReluctantOptionalUnary::Unbindfoo(const std::shared_ptr<test::Interface5>& theFoo)
{
  if (foo == theFoo)
  {
    foo = nullptr;
  }
}

} // namespaces
