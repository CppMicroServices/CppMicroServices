#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

void ServiceComponentDynamicGreedyOptionalUnary::Activate(
  const std::shared_ptr<ComponentContext>& /*ctxt*/)
{
}

void ServiceComponentDynamicGreedyOptionalUnary::Deactivate(
  const std::shared_ptr<ComponentContext>&)
{
}

std::string ServiceComponentDynamicGreedyOptionalUnary::ExtendedDescription()
{
  std::string result("ServiceComponentDynamicGreedyOptionalUnary ");
  result.append("depends on ");
  std::lock_guard<std::mutex> lock(fooMutex);
  if (foo) {
    result.append(foo->Description());
  }
  return result;
}

void ServiceComponentDynamicGreedyOptionalUnary::Bindfoo(
  const std::shared_ptr<test::Interface1>& theFoo)
{
  std::lock_guard<std::mutex> lock(fooMutex);
  if (foo != theFoo) {
    foo = theFoo;
  }
}

void ServiceComponentDynamicGreedyOptionalUnary::Unbindfoo(
  const std::shared_ptr<test::Interface1>& theFoo)
{
  std::lock_guard<std::mutex> lock(fooMutex);
  if (foo == theFoo) {
    foo = nullptr;
  }
}

} // namespaces
