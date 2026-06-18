#ifndef SERVICE_IMPL_DSTOI19_HPP
#define SERVICE_IMPL_DSTOI19_HPP

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponent19 : public test::Interface2
    {
      public:
        ServiceComponent19() = default;
        std::string ExtendedDescription() override;
        ~ServiceComponent19() = default;

        void Bindfoo(std::shared_ptr<test::Interface1> const&);
        void Unbindfoo(std::shared_ptr<test::Interface1> const&);

      private:
        std::shared_ptr<test::Interface1> foo;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSTOI19_HPP
