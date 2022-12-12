#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{

    class ServiceComponentDGMU : public test::Interface2
    {
      public:
        ServiceComponentDGMU() = default;
        std::string ExtendedDescription() override;
        ~ServiceComponentDGMU() = default;

        void Bindfoo(std::shared_ptr<test::Interface1> const&);
        void Unbindfoo(std::shared_ptr<test::Interface1> const&);

      private:
        std::shared_ptr<test::Interface1> foo;
    };

    class ServiceComponentDGOU : public test::Interface2
    {
      public:
        ServiceComponentDGOU() = default;
        std::string ExtendedDescription() override;
        ~ServiceComponentDGOU() = default;

        void Bindbar(std::shared_ptr<test::Interface1> const&);
        void Unbindbar(std::shared_ptr<test::Interface1> const&);

      private:
        std::shared_ptr<test::Interface1> foo;
    };

    class ServiceComponentFactory : public test::Interface2
    {
      public:
        ServiceComponentFactory() = default;
        std::string ExtendedDescription() override;
        ~ServiceComponentFactory() = default;

        void Bindfactory(std::shared_ptr<test::Interface1> const&);
        void Unbindfactory(std::shared_ptr<test::Interface1> const&);

      private:
        std::shared_ptr<test::Interface1> foo;
    };

} // namespace sample

#endif // _SERVICE_IMPL_HPP_
