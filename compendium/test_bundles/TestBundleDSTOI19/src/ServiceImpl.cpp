#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

std::string ServiceComponent19::ExtendedDescription()
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

void ServiceComponent19::Bindfoo(const std::shared_ptr<test::Interface1>& theFoo)
{
  if (foo != theFoo)
  {
    foo = theFoo;
  }
}

void ServiceComponent19::Unbindfoo(const std::shared_ptr<test::Interface1>& theFoo)
{
  if (foo == theFoo)
  {
    foo = nullptr;
  }
}

}
