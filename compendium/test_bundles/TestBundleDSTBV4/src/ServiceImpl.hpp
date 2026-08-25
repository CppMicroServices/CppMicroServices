#ifndef SERVICE_IMPL_DSTBV4_HPP
#define SERVICE_IMPL_DSTBV4_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class ServiceComponentBV41 : public test::Interface1
    {
      public:
        ServiceComponentBV41() = default;
        ~ServiceComponentBV41() override;
        std::string Description() override;
    };

    class ServiceComponentBV42 : public test::Interface1
    {
      public:
        ServiceComponentBV42() = default;
        ~ServiceComponentBV42() override;
        std::string Description() override;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSTBV4_HPP
