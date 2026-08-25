#ifndef SERVICE_IMPL_DSCA28_HPP
#define SERVICE_IMPL_DSCA28_HPP

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentCA28 : public test::CAInterface
    {
      public:
        ServiceComponentCA28(std::shared_ptr<cppmicroservices::AnyMap> const& props) : properties(props) {}
        void Modified(std::shared_ptr<ComponentContext> const& context,
                      std::shared_ptr<cppmicroservices::AnyMap> const& configuration);
        cppmicroservices::AnyMap GetProperties();
        ~ServiceComponentCA28() = default;

      private:
        std::mutex propertiesLock;
        std::shared_ptr<cppmicroservices::AnyMap> properties;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSCA28_HPP
