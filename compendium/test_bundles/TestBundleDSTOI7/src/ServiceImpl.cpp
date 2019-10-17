#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

void ServiceComponent7::Activate(const std::shared_ptr<ComponentContext>& /*ctxt*/)
{
//  foo = ctxt->LocateService<test::Interface1>("foo");
}
  
void ServiceComponent7::Deactivate(const std::shared_ptr<ComponentContext>&)
{
//  foo = nullptr;
}

std::string ServiceComponent7::ExtendedDescription()
{
  if(!foo)
  {
    throw std::runtime_error("Dependency not available");
  }
  std::string result(STRINGIZE(US_BUNDLE_NAME));
  result.append("depends on ");
  result.append(foo->Description());
  return result;
}

void ServiceComponent7::Bindfoo(const std::shared_ptr<test::Interface1>& theFoo)
{
  std::cout << "ENTER ServiceComponent7::Bindfoo()" << std::endl;
  if (foo != theFoo)
  {
    foo = theFoo;
  }
}

void ServiceComponent7::Unbindfoo(const std::shared_ptr<test::Interface1>& theFoo)
{
  std::cout << "ENTER ServiceComponent7::Unbindfoo()" << std::endl;
  if (foo == theFoo)
  {
    foo = nullptr;
  }
}

}
