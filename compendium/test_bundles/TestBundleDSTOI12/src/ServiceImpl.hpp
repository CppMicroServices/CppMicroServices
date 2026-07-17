#ifndef SERVICE_IMPL_DSTOI12_HPP
#define SERVICE_IMPL_DSTOI12_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class ServiceComponent12 : public test::Interface1
    {
      public:
        ServiceComponent12() = default;
        ~ServiceComponent12() override;
        std::string Description() override;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSTOI12_HPP
