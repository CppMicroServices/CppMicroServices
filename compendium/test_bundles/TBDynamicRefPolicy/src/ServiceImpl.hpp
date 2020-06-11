#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "TestInterfaces/Interfaces.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample {

class ServiceComponentDynamicReluctantMandatoryUnary final : public test::Interface2
{
public:
  ServiceComponentDynamicReluctantMandatoryUnary() = default;
  ~ServiceComponentDynamicReluctantMandatoryUnary() = default;
  std::string ExtendedDescription() override;
  
  void Activate(const std::shared_ptr<ComponentContext>&);
  void Deactivate(const std::shared_ptr<ComponentContext>&);

  void Bindfoo(const std::shared_ptr<test::Interface3>&);
  void Unbindfoo(const std::shared_ptr<test::Interface3>&);
private:
  std::shared_ptr<test::Interface3> foo;
};

class ServiceComponentDynamicGreedyMandatoryUnary final : public test::Interface2
{
public:
  ServiceComponentDynamicGreedyMandatoryUnary() = default;
  ~ServiceComponentDynamicGreedyMandatoryUnary() = default;
  virtual std::string ExtendedDescription() override;
  
  void Activate(const std::shared_ptr<ComponentContext>&);
  void Deactivate(const std::shared_ptr<ComponentContext>&);

  void Bindfoo(const std::shared_ptr<test::Interface4>&);
  void Unbindfoo(const std::shared_ptr<test::Interface4>&);
private:
  std::shared_ptr<test::Interface4> foo;
};

class ServiceComponentDynamicReluctantOptionalUnary final : public test::Interface2
{
public:
  ServiceComponentDynamicReluctantOptionalUnary() = default;
  ~ServiceComponentDynamicReluctantOptionalUnary() = default;
  virtual std::string ExtendedDescription() override;
  
  void Activate(const std::shared_ptr<ComponentContext>&);
  void Deactivate(const std::shared_ptr<ComponentContext>&);

  void Bindfoo(const std::shared_ptr<test::Interface5>&);
  void Unbindfoo(const std::shared_ptr<test::Interface5>&);
private:
  std::shared_ptr<test::Interface5> foo;
};

class ServiceComponentDynamicGreedyOptionalUnary final : public test::Interface2
{
public:
  ServiceComponentDynamicGreedyOptionalUnary() = default;
  ~ServiceComponentDynamicGreedyOptionalUnary() = default;
  virtual std::string ExtendedDescription() override;
  
  void Activate(const std::shared_ptr<ComponentContext>&);
  void Deactivate(const std::shared_ptr<ComponentContext>&);

  void Bindfoo(const std::shared_ptr<test::Interface6>&);
  void Unbindfoo(const std::shared_ptr<test::Interface6>&);
private:
  std::shared_ptr<test::Interface6> foo;
};

} // namespaces

#endif // _SERVICE_IMPL_HPP_
