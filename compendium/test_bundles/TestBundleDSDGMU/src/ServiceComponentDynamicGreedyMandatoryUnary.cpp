#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

void ServiceComponentDynamicGreedyMandatoryUnary::Activate(
  const std::shared_ptr<ComponentContext>& /*ctxt*/)
{
}

void ServiceComponentDynamicGreedyMandatoryUnary::Deactivate(
  const std::shared_ptr<ComponentContext>&)
{
}

std::string ServiceComponentDynamicGreedyMandatoryUnary::ExtendedDescription()
{
  std::lock_guard<std::mutex> lock(fooMutex);
  if (!foo) {
    throw std::runtime_error("Dependency not available");
  }
  std::string result("ServiceComponentDynamicGreedyMandatoryUnary ");
  result.append("depends on ");
  result.append(foo->Description());
  return result;
}

void ServiceComponentDynamicGreedyMandatoryUnary::Bindfoo(
  const std::shared_ptr<test::Interface1>& theFoo)
{
  std::lock_guard<std::mutex> lock(fooMutex);
  if (foo != theFoo) {
    foo = theFoo;
  }
}

void ServiceComponentDynamicGreedyMandatoryUnary::Unbindfoo(
  const std::shared_ptr<test::Interface1>& theFoo)
{
  std::lock_guard<std::mutex> lock(fooMutex);
  if (foo == theFoo) {
    foo = nullptr;
  }
}

} // namespaces
