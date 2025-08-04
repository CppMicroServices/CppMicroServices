#ifndef SERVICE_IMPL_HPP_
#define SERVICE_IMPL_HPP_

#include <TestInterfaces/Interfaces.hpp>
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceAImpl : public test::ServiceAInt
    {
      public:
        ServiceAImpl(std::shared_ptr<cppmicroservices::AnyMap> const& props,
                             const std::shared_ptr<test::ServiceBInt> interface1);
        void Modified(std::shared_ptr<ComponentContext> const& context,
                      std::shared_ptr<cppmicroservices::AnyMap> const& configuration);
        cppmicroservices::AnyMap GetProperties() override;
        ServiceAImpl() = default;
        ServiceAImpl(ServiceAImpl const&) = delete;
        ServiceAImpl(ServiceAImpl&&) = delete;
        ServiceAImpl& operator=(ServiceAImpl const&) = delete;
        ServiceAImpl& operator=(ServiceAImpl&&) = delete;
 
        ~ServiceAImpl() = default;

      private:
        std::mutex propertiesLock;
        cppmicroservices::AnyMap properties {};
        std::shared_ptr<test::ServiceBInt> serviceB {};
    };
} // namespace sample

#endif // SERVICE_IMPL_HPP_
