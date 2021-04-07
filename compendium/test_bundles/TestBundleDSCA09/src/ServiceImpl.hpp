#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "TestInterfaces/Interfaces.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
  class ServiceComponentCA09 : public test::CAInterface1
    {
    public:
      ServiceComponentCA09(std::shared_ptr<cppmicroservices::AnyMap> props,
                           const std::shared_ptr<test::Interface1> interface1);
      void Modified(
        const std::shared_ptr<ComponentContext>& context,
        const std::shared_ptr<cppmicroservices::AnyMap>& configuration);
      cppmicroservices::AnyMap GetProperties() override;
      bool isDependencyInjected() override;
 
      ~ServiceComponentCA09() = default;

    private:
      std::mutex propertiesLock;
      cppmicroservices::AnyMap properties;
      bool constructorHit;
    };
}

#endif // _SERVICE_IMPL_HPP_
