#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include <mutex>

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "TestInterfaces/Interfaces.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample {

class ServiceComponentDynamicGreedyOptionalUnary final : public test::Interface2
{
public:
  ServiceComponentDynamicGreedyOptionalUnary() = default;
  ~ServiceComponentDynamicGreedyOptionalUnary() = default;
  virtual std::string ExtendedDescription() override;
  
  void Activate(const std::shared_ptr<ComponentContext>&);
  void Deactivate(const std::shared_ptr<ComponentContext>&);

  void Bindfoo(const std::shared_ptr<test::Interface1>&);
  void Unbindfoo(const std::shared_ptr<test::Interface1>&);
private:
  std::shared_ptr<test::Interface1> foo;
  std::mutex fooMutex;
};

} // namespaces

#endif // _SERVICE_IMPL_HPP_
