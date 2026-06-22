#ifndef SERVICE_IMPL_DSCA12_HPP
#define SERVICE_IMPL_DSCA12_HPP

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentCA12 : public test::CAInterface
    {
      public:
        ServiceComponentCA12()
            : properties(std::make_shared<cppmicroservices::AnyMap>(
                cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS))
        {
        }

        void Modified(std::shared_ptr<ComponentContext> const& context,
                      std::shared_ptr<cppmicroservices::AnyMap> const& configuration);
        cppmicroservices::AnyMap GetProperties();

        ~ServiceComponentCA12() = default;

      private:
        std::mutex propertiesLock;
        std::shared_ptr<cppmicroservices::AnyMap> properties;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSCA12_HPP
