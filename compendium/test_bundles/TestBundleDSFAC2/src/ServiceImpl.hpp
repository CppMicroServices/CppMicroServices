#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceAImpl2 : public test::ServiceAInt
    {
      public:
        ServiceAImpl2(std::shared_ptr<cppmicroservices::AnyMap> const& props,
                      const std::shared_ptr<test::ServiceBInt> interface1,
                      const std::shared_ptr<test::ServiceCInt> interface2);
        void Modified(std::shared_ptr<ComponentContext> const& context,
                      std::shared_ptr<cppmicroservices::AnyMap> const& configuration);
        cppmicroservices::AnyMap GetProperties() override;
 
        ~ServiceAImpl2() = default;

      private:
        std::mutex propertiesLock;
        cppmicroservices::AnyMap properties;
        std::shared_ptr<test::ServiceBInt> serviceB;
        std::shared_ptr<test::ServiceCInt> serviceC;
    };
} // namespace sample

#endif // _SERVICE_IMPL_HPP_
