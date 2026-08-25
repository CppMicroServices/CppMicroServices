#ifndef SERVICE_IMPL_DSCA02_HPP
#define SERVICE_IMPL_DSCA02_HPP

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentCA02 : public test::CAInterface
    {
      public:
        ServiceComponentCA02(std::shared_ptr<cppmicroservices::AnyMap> const& props) : properties(props) {}
        void Modified(std::shared_ptr<ComponentContext> const& context,
                      std::shared_ptr<cppmicroservices::AnyMap> const& configuration);
        cppmicroservices::AnyMap GetProperties();
        ~ServiceComponentCA02() = default;

      private:
        std::mutex propertiesLock;
        std::shared_ptr<cppmicroservices::AnyMap> properties;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSCA02_HPP
