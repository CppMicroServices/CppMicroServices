#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentCA09 : public test::CAInterface1
    {
      public:
        ServiceComponentCA09(std::shared_ptr<cppmicroservices::AnyMap> const& props,
                             const std::shared_ptr<test::Interface1> interface1);
        void Modified(std::shared_ptr<ComponentContext> const& context,
                      std::shared_ptr<cppmicroservices::AnyMap> const& configuration);
        cppmicroservices::AnyMap GetProperties() override;
        bool isDependencyInjected() override;

        ~ServiceComponentCA09() = default;

      private:
        std::mutex propertiesLock;
        std::shared_ptr<cppmicroservices::AnyMap> properties;
        bool constructorHit;
    };
} // namespace sample

#endif // _SERVICE_IMPL_HPP_
