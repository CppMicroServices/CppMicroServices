#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/cm/ConfigurationAdmin.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentCA10 : public test::CAInterface
    {
      public:
        ServiceComponentCA10() = delete;
        ServiceComponentCA10(std::shared_ptr<cppmicroservices::AnyMap> const& props,
                             std::shared_ptr<cppmicroservices::service::cm::ConfigurationAdmin> const& CA);
        void Modified(std::shared_ptr<ComponentContext> const& context,
                      std::shared_ptr<cppmicroservices::AnyMap> const& configuration);
        cppmicroservices::AnyMap GetProperties() override;

        ~ServiceComponentCA10() = default;

      private:
        std::mutex propertiesLock;
        std::shared_ptr<cppmicroservices::AnyMap> properties;
        std::shared_ptr<cppmicroservices::service::cm::ConfigurationAdmin> CA;
    };
} // namespace sample

#endif // _SERVICE_IMPL_HPP_
