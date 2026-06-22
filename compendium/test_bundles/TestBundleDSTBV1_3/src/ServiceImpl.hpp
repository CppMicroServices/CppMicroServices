#ifndef SERVICE_IMPL_DSTBV1_3_HPP
#define SERVICE_IMPL_DSTBV1_3_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class ServiceComponentBV1_3 : public test::Interface1
    {
      public:
        ServiceComponentBV1_3() = default;
        ~ServiceComponentBV1_3() override;
        std::string Description() override;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSTBV1_3_HPP
