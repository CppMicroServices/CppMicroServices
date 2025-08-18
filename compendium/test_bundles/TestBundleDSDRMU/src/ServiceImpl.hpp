#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include <mutex>

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{

    class ServiceComponentDynamicReluctantMandatoryUnary final : public test::Interface2, public test::Interface4
    {
      public:
        ServiceComponentDynamicReluctantMandatoryUnary() = default;
        ~ServiceComponentDynamicReluctantMandatoryUnary() = default;
        std::string ExtendedDescription() override;
        bool isBound() override;
        void Activate(std::shared_ptr<ComponentContext> const&);
        void Deactivate(std::shared_ptr<ComponentContext> const&);

        void Bindfoo(std::shared_ptr<test::Interface1> const&);
        void Unbindfoo(std::shared_ptr<test::Interface1> const&);

      private:
        std::shared_ptr<test::Interface1> foo;
        std::mutex fooMutex;
    };

} // namespace sample

#endif // _SERVICE_IMPL_HPP_
