#ifndef SERVICE_IMPL_DSTBV1_HPP
#define SERVICE_IMPL_DSTBV1_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class ServiceComponentBV1 : public test::Interface1
    {
      public:
        ServiceComponentBV1() = default;
        ~ServiceComponentBV1() override;
        std::string Description() override;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSTBV1_HPP
