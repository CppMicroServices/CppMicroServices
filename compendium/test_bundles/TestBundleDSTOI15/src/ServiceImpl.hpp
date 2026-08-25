#ifndef SERVICE_IMPL_DSTOI15_HPP
#define SERVICE_IMPL_DSTOI15_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class ServiceComponent15 : public test::Interface1
    {
      public:
        ServiceComponent15() = default;
        ~ServiceComponent15() override;
        std::string Description() override;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSTOI15_HPP
