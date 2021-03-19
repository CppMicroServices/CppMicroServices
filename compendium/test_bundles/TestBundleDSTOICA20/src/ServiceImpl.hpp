#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "TestInterfaces/Interfaces.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
  class ServiceComponent20 : public test::CAInterface
    {
    public:
      ServiceComponent20(std::shared_ptr<cppmicroservices::AnyMap> props) 
          : properties(*props)
      {
      }
      void Modified(
        const std::shared_ptr<ComponentContext>& context,
        const std::shared_ptr<cppmicroservices::AnyMap>& configuration);
      cppmicroservices::AnyMap GetProperties();
 
      ~ServiceComponent20() = default;

    private:
      std::mutex propertiesLock;
      cppmicroservices::AnyMap properties;
    };
}

#endif // _SERVICE_IMPL_HPP_
