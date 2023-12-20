#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

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

#endif // _SERVICE_IMPL_HPP_
