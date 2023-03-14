#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

void ServiceComponentDynamicReluctantOptionalUnary::Activate(
  const std::shared_ptr<ComponentContext>& /*ctxt*/)
{
}

void ServiceComponentDynamicReluctantOptionalUnary::Deactivate(
  const std::shared_ptr<ComponentContext>&)
{
}

std::string ServiceComponentDynamicReluctantOptionalUnary::ExtendedDescription()
{
  std::string result("ServiceComponentDynamicReluctantOptionalUnary ");
  result.append("depends on ");
  std::lock_guard<std::mutex> lock(fooMutex);
  if (foo) {
    result.append(foo->Description());
  }
  return result;
}

void ServiceComponentDynamicReluctantOptionalUnary::Bindfoo(
  const std::shared_ptr<test::Interface1>& theFoo)
{
  std::lock_guard<std::mutex> lock(fooMutex);
  if (foo != theFoo) {
    foo = theFoo;
  }
}

void ServiceComponentDynamicReluctantOptionalUnary::Unbindfoo(
  const std::shared_ptr<test::Interface1>& theFoo)
{
  std::lock_guard<std::mutex> lock(fooMutex);
  if (foo == theFoo) {
    foo = nullptr;
  }
}

} // namespaces
