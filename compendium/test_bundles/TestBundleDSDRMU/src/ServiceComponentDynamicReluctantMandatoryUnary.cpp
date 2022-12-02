#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

void ServiceComponentDynamicReluctantMandatoryUnary::Activate(
  const std::shared_ptr<ComponentContext>& /*ctxt*/)
{
}

void ServiceComponentDynamicReluctantMandatoryUnary::Deactivate(
  const std::shared_ptr<ComponentContext>&)
{
}

std::string
ServiceComponentDynamicReluctantMandatoryUnary::ExtendedDescription()
{
  std::lock_guard<std::mutex> lock(fooMutex);
  if (!foo) {
    throw std::runtime_error("Dependency not available");
  }
  std::string result("ServiceComponentDynamicReluctantMandatoryUnary ");
  result.append("depends on ");
  result.append(foo->Description());
  return result;
}

void ServiceComponentDynamicReluctantMandatoryUnary::Bindfoo(
  const std::shared_ptr<test::Interface1>& theFoo)
{
  std::lock_guard<std::mutex> lock(fooMutex);
  if (foo != theFoo) {
    foo = theFoo;
  }
}

void ServiceComponentDynamicReluctantMandatoryUnary::Unbindfoo(
  const std::shared_ptr<test::Interface1>& theFoo)
{
  std::lock_guard<std::mutex> lock(fooMutex);
  if (foo == theFoo) {
    foo = nullptr;
  }
}

} // namespaces
