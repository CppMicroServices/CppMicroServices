#ifndef SERVICE_IMPL_DSTOI21_HPP
#define SERVICE_IMPL_DSTOI21_HPP

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;
using BundleContext = cppmicroservices::BundleContext;

namespace sample
{

    class ServiceComponent21 : public test::TestInitialization
    {
      public:
        ServiceComponent21() = default;
        void Activate(std::shared_ptr<ComponentContext> const&);
        void Deactivate(std::shared_ptr<ComponentContext> const&);
        ~ServiceComponent21() = default;

        std::vector<BundleContext> GetContexts(void);

      private:
        BundleContext providedCtx;
    };

} // namespace sample

#endif // SERVICE_IMPL_DSTOI21_HPP
