#ifndef SERVICE_IMPL_DSCA05A_HPP
#define SERVICE_IMPL_DSCA05A_HPP

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentCA05a : public test::CAInterface
    {
      public:
        ServiceComponentCA05a(std::shared_ptr<cppmicroservices::AnyMap> const& props) : properties(props) {}

        cppmicroservices::AnyMap GetProperties();

        ~ServiceComponentCA05a() = default;

      private:
        std::mutex propertiesLock;
        std::shared_ptr<cppmicroservices::AnyMap> properties;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSCA05A_HPP
