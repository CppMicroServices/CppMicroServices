#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample {

class ServiceComponentDGMU : public test::Interface2
{
public:
  ServiceComponentDGMU() = default;
  std::string ExtendedDescription() override;
  ~ServiceComponentDGMU() = default;

  void Bindfoo(const std::shared_ptr<test::Interface1>&);
  void Unbindfoo(const std::shared_ptr<test::Interface1>&);

private:
  std::shared_ptr<test::Interface1> foo;
};

class ServiceComponentDGOU : public test::Interface2
{
public:
  ServiceComponentDGOU() = default;
  std::string ExtendedDescription() override;
  ~ServiceComponentDGOU() = default;

  void Bindbar(const std::shared_ptr<test::Interface1>&);
  void Unbindbar(const std::shared_ptr<test::Interface1>&);

private:
  std::shared_ptr<test::Interface1> foo;
};

class ServiceComponentFactory : public test::Interface2
{
public:
  ServiceComponentFactory() = default;
  std::string ExtendedDescription() override;
  ~ServiceComponentFactory() = default;

  void Bindfactory(const std::shared_ptr<test::Interface1>&);
  void Unbindfactory(const std::shared_ptr<test::Interface1>&);

private:
  std::shared_ptr<test::Interface1> foo;
};

} // namespaces

#endif // _SERVICE_IMPL_HPP_
