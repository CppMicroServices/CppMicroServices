#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "TestInterfaces/Interfaces.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample {
  class ServiceComponent10 : public test::LifeCycleValidation {
  public:
    ServiceComponent10() = default;
    ~ServiceComponent10() override;
    void Activate(const std::shared_ptr<ComponentContext>& context);
    void Deactivate(const std::shared_ptr<ComponentContext>& context);
    bool IsActivated() override { return activated; };
    bool IsDeactivated() override { return deactivated; };
  private:
    bool activated;
    bool deactivated;
  };
}

#endif // _SERVICE_IMPL_HPP_
