#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "TestInterfaces/Interfaces.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample {
  class ServiceComponentDSSLE2 : public test::InterfaceSLE {
  public:
    ServiceComponentDSSLE2() = default;
    ~ServiceComponentDSSLE2() override;
    void Activate(const std::shared_ptr<ComponentContext>& context);
    void Deactivate(const std::shared_ptr<ComponentContext>& context);
  };
}

#endif // _SERVICE_IMPL_HPP_
