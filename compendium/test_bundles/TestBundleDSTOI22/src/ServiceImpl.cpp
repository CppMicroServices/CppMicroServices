#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

std::string ServiceComponent22::ExtendedDescription()
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

void ServiceComponent22::Bindfoo(const std::shared_ptr<test::Interface1>& theFoo)
{
  throw std::runtime_error("throw from bind method");
}

void ServiceComponent22::Unbindfoo(const std::shared_ptr<test::Interface1>& theFoo)
{
  throw std::runtime_error("throw from unbind method");
}

} // namespaces
