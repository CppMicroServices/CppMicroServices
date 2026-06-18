#ifndef SERVICE_IMPL_DSTBV3_HPP
#define SERVICE_IMPL_DSTBV3_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class ServiceComponentBV3 : public test::Interface1
    {
      public:
        ServiceComponentBV3() = default;
        ~ServiceComponentBV3() override;
        std::string Description() override;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSTBV3_HPP
