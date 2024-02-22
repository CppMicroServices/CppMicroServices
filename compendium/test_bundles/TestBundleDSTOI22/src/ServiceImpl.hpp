#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;
using BundleContext = cppmicroservices::BundleContext;

namespace sample
{

    class ServiceComponent22 : public test::TestInitialization
    {
      public:
        ServiceComponent22() = default;
        void Activate(std::shared_ptr<ComponentContext> const&);
        void Deactivate(std::shared_ptr<ComponentContext> const&);
        ~ServiceComponent22() = default;

        std::vector<BundleContext> GetContexts(void);

      private:
        BundleContext providedCtx;
    };

    extern BundleContext activatorProvidedCtx;

} // namespace sample

#endif // _SERVICE_IMPL_HPP_
