#ifndef SERVICE_IMPL_DSCA29_HPP
#define SERVICE_IMPL_DSCA29_HPP

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentCA29 : public test::CAInterface
    {
      public:
        ServiceComponentCA29(std::shared_ptr<cppmicroservices::AnyMap> const& props);
        cppmicroservices::AnyMap GetProperties();
        ~ServiceComponentCA29() = default;

      private:
        std::mutex propertiesLock;
        std::shared_ptr<cppmicroservices::AnyMap> properties;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSCA29_HPP
