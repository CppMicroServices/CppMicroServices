#ifndef SERVICE_IMPL_DSTOI22_HPP
#define SERVICE_IMPL_DSTOI22_HPP

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

#endif // SERVICE_IMPL_DSTOI22_HPP
