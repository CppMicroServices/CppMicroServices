#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include <mutex>

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "cppmicroservices/servicecomponent/UserDefinedMethodVerification.hpp"
using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{

    class ServiceComponent_userMethodStaticChecks
        : public test::Interface2
        , cppmicroservices::service::component::UserDefinedMethodVerification<ServiceComponent_userMethodStaticChecks>
    {
      public:
        ServiceComponent_userMethodStaticChecks() = default;
        ~ServiceComponent_userMethodStaticChecks() = default;
        std::string ExtendedDescription() override;

        void Bindfoo(std::shared_ptr<test::DSGraph01> const& theFoo);
        void Unbindfoo(std::shared_ptr<test::DSGraph01> const& theFoo);

        void Modified(std::shared_ptr<ComponentContext> const& context,
                      std::shared_ptr<cppmicroservices::AnyMap> const& configuration);
        void Activate(std::shared_ptr<ComponentContext> const& context);
        void Deactivate(std::shared_ptr<ComponentContext> const& context);

      private:
        std::shared_ptr<test::DSGraph01> foo;
        std::mutex fooMutex;
    };

    class ServiceComponent_userDSMethodStaticChecks
        : public test::Interface2
        , cppmicroservices::service::component::UserDefinedDSMethodVerification<
              ServiceComponent_userDSMethodStaticChecks>
    {
      public:
        ServiceComponent_userDSMethodStaticChecks() = default;
        ~ServiceComponent_userDSMethodStaticChecks() = default;
        std::string ExtendedDescription() override;

        void Bindfoo(std::shared_ptr<test::DSGraph02> const& theFoo);
        void Unbindfoo(std::shared_ptr<test::DSGraph02> const& theFoo);

        void Activate(std::shared_ptr<ComponentContext> const& context);
        void Deactivate(std::shared_ptr<ComponentContext> const& context);

      private:
        std::shared_ptr<test::DSGraph02> foo;
        std::mutex fooMutex;
    };

    class ServiceComponent_userCAMethodStaticChecks
        : public test::Interface2
        , cppmicroservices::service::component::UserDefinedCAMethodVerification<
              ServiceComponent_userCAMethodStaticChecks>
    {
      public:
        ServiceComponent_userCAMethodStaticChecks() = default;
        ~ServiceComponent_userCAMethodStaticChecks() = default;
        std::string ExtendedDescription() override;

        void Modified(std::shared_ptr<ComponentContext> const& context,
                      std::shared_ptr<cppmicroservices::AnyMap> const& configuration);

        void Bindfoo(std::shared_ptr<test::DSGraph03> const& theFoo);
        void Unbindfoo(std::shared_ptr<test::DSGraph03> const& theFoo);

      private:
        std::shared_ptr<test::DSGraph03> foo;
        std::mutex fooMutex;
    };

} // namespace sample

#endif // _SERVICE_IMPL_HPP_
