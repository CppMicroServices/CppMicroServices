#ifndef SERVICE_IMPL_HPP_
#define SERVICE_IMPL_HPP_

#include <atomic>
#include <mutex>
#include <vector>
#include <memory>
#include <string>

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{

    class ServiceComponentDynamicReluctantOptionalMultiple final : public test::Interface2
    {
      public:
        ServiceComponentDynamicReluctantOptionalMultiple(std::shared_ptr<test::Interface1> const& dep);
        ~ServiceComponentDynamicReluctantOptionalMultiple() = default;

        std::string ExtendedDescription() override;

        void Activate(std::shared_ptr<ComponentContext> const&);
        void Deactivate(std::shared_ptr<ComponentContext> const&);

        void BindoptionalDeps(std::shared_ptr<test::Interface3> const&);
        void UnbindoptionalDeps(std::shared_ptr<test::Interface3> const&);

      private:
        std::shared_ptr<test::Interface1> staticDep_;
        std::vector<std::shared_ptr<test::Interface3>> optionalDeps_;
        std::mutex optionalDepsMutex_;
        std::atomic<int> bindCount_ {0};
        std::atomic<int> nullBindCount_ {0};
        std::atomic<int> unbindCount_ {0};

        // Static so it survives component instance destruction during deactivation
        // (UnbindReferences is called as part of DestroyComponentInstances)
        static std::atomic<int> nullUnbindCount_;
    };

} // namespace sample

#endif // SERVICE_IMPL_HPP_
