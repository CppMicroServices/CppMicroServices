#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "TestInterfaces/Interfaces.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
  class ServiceComponentCA17 : public test::CAInterface
    {
    public:
      ServiceComponentCA17(const std::shared_ptr<cppmicroservices::AnyMap>& props) 
          : properties(*props)
      {
      }

      cppmicroservices::AnyMap GetProperties();

      ~ServiceComponentCA17() = default;

    private:
      std::mutex propertiesLock;
      cppmicroservices::AnyMap properties;
    };
}

#endif // _SERVICE_IMPL_HPP_
