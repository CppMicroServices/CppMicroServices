#ifndef SERVICE_IMPL_DSMIPROVIDER_HPP
#define SERVICE_IMPL_DSMIPROVIDER_HPP

#include <string>

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    // A single component that provides two independent interfaces. Because
    // test::MultiInterfaceA and test::MultiInterfaceB are unrelated bases, the
    // MultiInterfaceB sub-object lives at a non-zero offset within the object,
    // so static_cast<test::MultiInterfaceB*>(this) differs from
    // static_cast<test::MultiInterfaceA*>(this).
    class MultiInterfaceProvider final
        : public test::MultiInterfaceA
        , public test::MultiInterfaceB
    {
      public:
        MultiInterfaceProvider() = default;
        ~MultiInterfaceProvider() override = default;

        std::string WhoA() override;
        std::string WhoB() override;
    };

} // namespace sample

#endif // SERVICE_IMPL_DSMIPROVIDER_HPP
