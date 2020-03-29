#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "TestInterfaces/Interfaces.hpp"

namespace sample {
  class ServiceComponent : public test::Interface1 {
  public:
    ServiceComponent() = default;
    ~ServiceComponent() override;
    void Activate(const std::shared_ptr<ComponentContext>& context);
    void Deactivate(const std::shared_ptr<ComponentContext>& context);
  };
}

#endif // _SERVICE_IMPL_HPP_
