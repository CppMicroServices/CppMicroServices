#ifndef SERVICE_IMPL_DSTOI14_HPP
#define SERVICE_IMPL_DSTOI14_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class ServiceComponent14 : public test::Interface1
    {
      public:
        ServiceComponent14() = default;
        ~ServiceComponent14() override;
        std::string Description() override;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSTOI14_HPP
