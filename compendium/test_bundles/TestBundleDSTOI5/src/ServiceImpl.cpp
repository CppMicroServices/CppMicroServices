#include "ServiceImpl.hpp"

namespace sample {

void ServiceComponent5::Activate(const std::shared_ptr<ComponentContext>& ctxt)
{
  foo = ctxt->LocateService<test::Interface1>("foo");
}
  
void ServiceComponent5::Deactivate(const std::shared_ptr<ComponentContext>&)
{
  foo = nullptr;
}

std::string ServiceComponent5::ExtendedDescription()
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

void ServiceComponent5::Bindfoo(const std::shared_ptr<test::Interface1>& theFoo)
{
  if (foo != theFoo)
  {
    foo = theFoo;
  }
}

void ServiceComponent5::Unbindfoo(const std::shared_ptr<test::Interface1>& theFoo)
{
  if (foo == theFoo)
  {
    foo = nullptr;
  }
}

}
