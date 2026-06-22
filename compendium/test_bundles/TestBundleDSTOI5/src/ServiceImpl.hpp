#ifndef SERVICE_IMPL_DSTOI5_HPP
#define SERVICE_IMPL_DSTOI5_HPP

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponent5 : public test::Interface2
    {
      public:
        ServiceComponent5() = default;
        std::string ExtendedDescription() override;
        void Activate(std::shared_ptr<ComponentContext> const&);
        void Deactivate(std::shared_ptr<ComponentContext> const&);
        ~ServiceComponent5() = default;

        void Bindfoo(std::shared_ptr<test::Interface1> const&);
        void Unbindfoo(std::shared_ptr<test::Interface1> const&);

      private:
        std::shared_ptr<test::Interface1> foo;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSTOI5_HPP
