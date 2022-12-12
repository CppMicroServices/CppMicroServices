#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentCA07 : public test::CAInterface
    {
      public:
        ServiceComponentCA07()
            : properties(std::make_shared<cppmicroservices::AnyMap>(
                cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS))
        {
        }

        void Modified(std::shared_ptr<ComponentContext> const& context,
                      std::shared_ptr<cppmicroservices::AnyMap> const& configuration);
        cppmicroservices::AnyMap GetProperties();

        ~ServiceComponentCA07() = default;

      private:
        std::mutex propertiesLock;
        std::shared_ptr<cppmicroservices::AnyMap> properties;
    };
} // namespace sample

#endif // _SERVICE_IMPL_HPP_
