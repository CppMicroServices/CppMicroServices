#ifndef SERVICE_IMPL_DSTBV5_HPP
#define SERVICE_IMPL_DSTBV5_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class ServiceComponentBV5 : public test::Interface1
    {
      public:
        ServiceComponentBV5() = default;
        ~ServiceComponentBV5() override;
        std::string Description() override;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSTBV5_HPP
