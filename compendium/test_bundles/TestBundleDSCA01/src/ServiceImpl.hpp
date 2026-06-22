#ifndef SERVICE_IMPL_DSCA01_HPP
#define SERVICE_IMPL_DSCA01_HPP

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentCA01 : public test::CAInterface
    {
      public:
        ServiceComponentCA01(std::shared_ptr<cppmicroservices::AnyMap> const& props) : properties(props) {}
        void Modified(std::shared_ptr<ComponentContext> const& context,
                      std::shared_ptr<cppmicroservices::AnyMap> const& configuration);
        cppmicroservices::AnyMap GetProperties();
        ~ServiceComponentCA01() = default;

      private:
        std::mutex propertiesLock;
        std::shared_ptr<cppmicroservices::AnyMap> properties;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSCA01_HPP
