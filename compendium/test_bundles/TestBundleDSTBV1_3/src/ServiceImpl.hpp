#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

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

#endif // _SERVICE_IMPL_HPP_
