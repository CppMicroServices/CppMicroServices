#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentBV1_4 : public test::CAInterface
    {
      public:
        ServiceComponentBV1_4(std::shared_ptr<cppmicroservices::AnyMap> const& props) : properties(props) {}
        void Modified(std::shared_ptr<ComponentContext> const& context,
                      std::shared_ptr<cppmicroservices::AnyMap> const& configuration);
        cppmicroservices::AnyMap GetProperties();
        ~ServiceComponentBV1_4() = default;

      private:
        std::mutex propertiesLock;
        std::shared_ptr<cppmicroservices::AnyMap> properties;
    };
} // namespace sample

#endif // _SERVICE_IMPL_HPP_
