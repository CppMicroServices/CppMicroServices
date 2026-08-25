#ifndef SERVICE_IMPL_DSTOI9_HPP
#define SERVICE_IMPL_DSTOI9_HPP

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponent9 : public test::LifeCycleValidation
    {
      public:
        ServiceComponent9() = default;
        ~ServiceComponent9() override = default;
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

#endif // SERVICE_IMPL_DSTOI9_HPP
