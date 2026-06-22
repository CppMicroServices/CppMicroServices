#ifndef SERVICE_IMPL_DSTOI1_HPP
#define SERVICE_IMPL_DSTOI1_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class ServiceComponent : public test::Interface1
    {
      public:
        ServiceComponent() = default;
        ~ServiceComponent() override;
        std::string Description() override;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSTOI1_HPP
