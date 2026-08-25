#ifndef SERVICE_IMPL_DSCA30_HPP
#define SERVICE_IMPL_DSCA30_HPP

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentCA30 : public test::CAInterface
    {
      public:
        ServiceComponentCA30()
            : properties(std::make_shared<cppmicroservices::AnyMap>(
                cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS))
        {
        }

        void Modified(std::shared_ptr<ComponentContext> const& context,
                      std::shared_ptr<cppmicroservices::AnyMap> const& configuration);
        cppmicroservices::AnyMap GetProperties();

        ~ServiceComponentCA30() = default;

      private:
        std::mutex propertiesLock;
        std::shared_ptr<cppmicroservices::AnyMap> properties;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSCA30_HPP
