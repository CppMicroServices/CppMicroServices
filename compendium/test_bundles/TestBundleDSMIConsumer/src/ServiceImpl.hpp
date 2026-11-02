#ifndef SERVICE_IMPL_DSMICONSUMER_HPP
#define SERVICE_IMPL_DSMICONSUMER_HPP

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    // Consumer component with a dynamic, reluctant, 0..n reference to
    // test::MultiInterfaceA. When a provider service is registered at runtime,
    // the DS runtime binds it to this reference via BindmiA. The component
    // provides test::MultiInterfaceProbe so a test can observe what was bound.
    class MultiInterfaceConsumer final : public test::MultiInterfaceProbe
    {
      public:
        MultiInterfaceConsumer() = default;
        ~MultiInterfaceConsumer() override = default;

        std::size_t BoundCount() override;
        std::string BoundIdentity() override;

        void BindmiA(std::shared_ptr<test::MultiInterfaceA> const&);
        void UnbindmiA(std::shared_ptr<test::MultiInterfaceA> const&);

      private:
        std::vector<std::shared_ptr<test::MultiInterfaceA>> boundA_;
        std::mutex boundAMutex_;
    };

} // namespace sample

#endif // SERVICE_IMPL_DSMICONSUMER_HPP
