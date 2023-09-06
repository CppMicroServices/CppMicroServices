#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentBV1_5 : public test::CAInterface
    {
      public:
        ServiceComponentBV1_5(std::shared_ptr<cppmicroservices::AnyMap> const& props) : properties(props) {}

        cppmicroservices::AnyMap GetProperties();

        ~ServiceComponentBV1_5() = default;

      private:
        std::mutex propertiesLock;
        std::shared_ptr<cppmicroservices::AnyMap> properties;
    };
} // namespace sample

#endif // _SERVICE_IMPL_HPP_
