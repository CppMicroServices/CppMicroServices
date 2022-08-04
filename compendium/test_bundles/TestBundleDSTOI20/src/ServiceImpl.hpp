#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample {
class ServiceComponent : public test::Interface4
{
public:
  ServiceComponent();
  ~ServiceComponent() override;
  std::shared_ptr<test::LifeCycleValidation> GetService() override;

private:
  std::shared_ptr<test::LifeCycleValidation> service;
};
}

#endif // _SERVICE_IMPL_HPP_
