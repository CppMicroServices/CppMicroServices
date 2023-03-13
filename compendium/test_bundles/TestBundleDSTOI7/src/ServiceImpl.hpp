#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{

    class ServiceComponent7 : public test::Interface2
    {
      public:
        ServiceComponent7() = default;
        std::string ExtendedDescription() override;
        void Activate(std::shared_ptr<ComponentContext> const&);
        void Deactivate(std::shared_ptr<ComponentContext> const&);
        ~ServiceComponent7() = default;

        void Bindfoo(std::shared_ptr<test::Interface1> const&);
        void Unbindfoo(std::shared_ptr<test::Interface1> const&);

      private:
        std::shared_ptr<test::Interface1> foo;
    };

} // namespace sample

#endif // _SERVICE_IMPL_HPP_
