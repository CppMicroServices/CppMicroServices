#ifndef _SERVICE_IMPL_CA29_HPP_
#define _SERVICE_IMPL_CA29_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <atomic>
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentCA29 : public test::CAInterface
    {
      public:
        ServiceComponentCA29(std::shared_ptr<cppmicroservices::AnyMap> const& props);
        cppmicroservices::AnyMap GetProperties();
        ~ServiceComponentCA29() = default;

        static std::atomic<int> constructionCount;
        static std::atomic<int> missingPropertyCount;

      private:
        std::mutex propertiesLock;
        std::shared_ptr<cppmicroservices::AnyMap> properties;
    };
} // namespace sample

#endif // _SERVICE_IMPL_CA29_HPP_
