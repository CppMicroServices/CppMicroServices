#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

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

#endif // _SERVICE_IMPL_HPP_
