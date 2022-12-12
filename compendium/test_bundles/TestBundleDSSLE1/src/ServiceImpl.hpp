#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentDSSLE1 : public test::Interface1
    {
      public:
        ServiceComponentDSSLE1() = default;
        ~ServiceComponentDSSLE1() override;
        std::string Description() override;
        void Activate(std::shared_ptr<ComponentContext> const& context);
        void Deactivate(std::shared_ptr<ComponentContext> const& context);
    };
} // namespace sample

#endif // _SERVICE_IMPL_HPP_
