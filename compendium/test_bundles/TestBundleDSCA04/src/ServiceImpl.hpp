#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentCA04 : public test::CAInterface
    {
      public:
        ServiceComponentCA04(std::shared_ptr<cppmicroservices::AnyMap> const& props) : properties(props) {}

        cppmicroservices::AnyMap GetProperties();

        ~ServiceComponentCA04() = default;

      private:
        std::mutex propertiesLock;
        std::shared_ptr<cppmicroservices::AnyMap> properties;
    };
} // namespace sample

#endif // _SERVICE_IMPL_HPP_
