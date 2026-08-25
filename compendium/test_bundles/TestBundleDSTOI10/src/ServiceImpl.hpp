#ifndef SERVICE_IMPL_DSTOI10_HPP
#define SERVICE_IMPL_DSTOI10_HPP

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

#endif // SERVICE_IMPL_DSTOI10_HPP
