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
                     std::shared_ptr<test::ServiceBInt> const interface1);
        void Modified(std::shared_ptr<ComponentContext> const& context,
                      std::shared_ptr<cppmicroservices::AnyMap> const& configuration);
        cppmicroservices::AnyMap GetProperties() override;
        ServiceAImpl() = default;
        ServiceAImpl(ServiceAImpl const&) = delete;
        ServiceAImpl(ServiceAImpl&&) = delete;
        ServiceAImpl& operator=(ServiceAImpl const&) = delete;
        ServiceAImpl& operator=(ServiceAImpl&&) = delete;

        ~ServiceAImpl() = default;

        [[nodiscard]] void*
        GetRefAddr() const override
        {
            return static_cast<void*>(serviceB.get());
        }

      private:
        std::mutex propertiesLock;
        cppmicroservices::AnyMap properties {};
        std::shared_ptr<test::ServiceBInt> serviceB {};
    };
    class ServiceAImpl2 : public test::ServiceAInt
    {
      public:
        ServiceAImpl2(std::shared_ptr<cppmicroservices::AnyMap> const& props,
                      std::shared_ptr<test::ServiceBInt> const interface1);
        ServiceAImpl2() = default;
        ServiceAImpl2(ServiceAImpl const&) = delete;
        ServiceAImpl2(ServiceAImpl&&) = delete;
        ServiceAImpl2& operator=(ServiceAImpl const&) = delete;
        ServiceAImpl2& operator=(ServiceAImpl&&) = delete;

        ~ServiceAImpl2() = default;

        void
        Modified(std::shared_ptr<ComponentContext> const&, std::shared_ptr<cppmicroservices::AnyMap> const&)
        {
        }

        cppmicroservices::AnyMap
        GetProperties() override
        {
            return {};
        }

        [[nodiscard]] void*
        GetRefAddr() const override
        {
            return static_cast<void*>(serviceB.get());
        }

      private:
        std::shared_ptr<test::ServiceBInt> serviceB {};
    };
    class ServiceAImpl3 : public test::ServiceAInt
    {
      public:
        ServiceAImpl3(std::shared_ptr<cppmicroservices::AnyMap> const& props,
                      std::shared_ptr<test::ServiceCInt> const interface1);
        ServiceAImpl3() = default;
        ServiceAImpl3(ServiceAImpl const&) = delete;
        ServiceAImpl3(ServiceAImpl&&) = delete;
        ServiceAImpl3& operator=(ServiceAImpl const&) = delete;
        ServiceAImpl3& operator=(ServiceAImpl&&) = delete;
        ~ServiceAImpl3() = default;

        void
        Modified(std::shared_ptr<ComponentContext> const&, std::shared_ptr<cppmicroservices::AnyMap> const&)
        {
        }
        cppmicroservices::AnyMap
        GetProperties() override
        {
            return {};
        }

        [[nodiscard]] void*
        GetRefAddr() const override
        {
            return static_cast<void*>(serviceC.get());
        }

      private:
        std::shared_ptr<test::ServiceCInt> serviceC {};
    };
    class ServiceBImpl2 : public test::ServiceBInt
    {
      public:
        ServiceBImpl2(std::shared_ptr<cppmicroservices::AnyMap> const&) {}
        ServiceBImpl2() = default;
        ServiceBImpl2(ServiceBImpl2 const&) = delete;
        ServiceBImpl2(ServiceBImpl2&&) = delete;
        ServiceBImpl2& operator=(ServiceBImpl2 const&) = delete;
        ServiceBImpl2& operator=(ServiceBImpl2&&) = delete;
        ~ServiceBImpl2() = default;

        cppmicroservices::AnyMap
        GetProperties() override
        {
            return {};
        }

        void
        Modified(std::shared_ptr<ComponentContext> const&, std::shared_ptr<cppmicroservices::AnyMap> const&)
        {
        }
    };
    class ServiceCImpl2 : public test::ServiceCInt
    {
      public:
        ServiceCImpl2(std::shared_ptr<cppmicroservices::AnyMap> const&) {}
        ServiceCImpl2() = default;
        ServiceCImpl2(ServiceCImpl2 const&) = delete;
        ServiceCImpl2(ServiceCImpl2&&) = delete;
        ServiceCImpl2& operator=(ServiceCImpl2 const&) = delete;
        ServiceCImpl2& operator=(ServiceCImpl2&&) = delete;
        ~ServiceCImpl2() = default;

        cppmicroservices::AnyMap
        GetProperties() override
        {
            return {};
        }

        void
        Modified(std::shared_ptr<ComponentContext> const&, std::shared_ptr<cppmicroservices::AnyMap> const&)
        {
        }
    };

} // namespace sample

#endif // SERVICE_IMPL_HPP_
