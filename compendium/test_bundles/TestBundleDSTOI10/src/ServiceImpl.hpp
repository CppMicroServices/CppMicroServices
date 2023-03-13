#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponent10 : public test::LifeCycleValidation
    {
      public:
        ServiceComponent10() = default;
        ~ServiceComponent10() override;
        void Activate(std::shared_ptr<ComponentContext> const& context);
        void Deactivate(std::shared_ptr<ComponentContext> const& context);
        bool
        IsActivated() override
        {
            return activated;
        };
        bool
        IsDeactivated() override
        {
            return deactivated;
        };

      private:
        bool activated;
        bool deactivated;
    };
} // namespace sample

#endif // _SERVICE_IMPL_HPP_
