#ifndef SERVICE_IMPL_DSTBV2_HPP
#define SERVICE_IMPL_DSTBV2_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class ServiceComponentBV2 : public test::Interface1
    {
      public:
        ServiceComponentBV2() = default;
        ~ServiceComponentBV2() override;
        std::string Description() override;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSTBV2_HPP
