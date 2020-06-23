#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

std::string ServiceComponentDGMU::ExtendedDescription()
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

void ServiceComponentDGMU::Bindfoo(
  const std::shared_ptr<test::Interface1>& )
{
  throw std::runtime_error("throw from bind method");
}

void ServiceComponentDGMU::Unbindfoo(
  const std::shared_ptr<test::Interface1>& )
{
  throw std::runtime_error("throw from unbind method");
}

std::string ServiceComponentDGOU::ExtendedDescription()
{
  if (!foo) {
    throw std::runtime_error("Dependency not available");
  }
  std::string result(STRINGIZE(US_BUNDLE_NAME));
  result.append("depends on ");
  result.append(foo->Description());
  return result;
}

void ServiceComponentDGOU::Bindbar(
  const std::shared_ptr<test::Interface1>& )
{
  throw std::runtime_error("throw from bind method");
}

void ServiceComponentDGOU::Unbindbar(
  const std::shared_ptr<test::Interface1>& )
{

  throw std::runtime_error("throw from unbind method");
}

std::string ServiceComponentFactory::ExtendedDescription()
{
  if (!foo) {
    throw std::runtime_error("Dependency not available");
  }
  std::string result(STRINGIZE(US_BUNDLE_NAME));
  result.append("depends on ");
  result.append(foo->Description());
  return result;
}

void ServiceComponentFactory::Bindfactory(
  const std::shared_ptr<test::Interface1>& )
{
  throw std::runtime_error("throw from bind method");
}

void ServiceComponentFactory::Unbindfactory(
  const std::shared_ptr<test::Interface1>& )
{

  throw std::runtime_error("throw from unbind method");
}

} // namespaces
