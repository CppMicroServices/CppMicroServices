#ifndef SERVICE_IMPL_DSTOI16_HPP
#define SERVICE_IMPL_DSTOI16_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class ServiceComponent16
        : public test::Interface1
        , public test::Interface2
    {
      public:
        std::string Description() override;
        std::string ExtendedDescription() override;
        ~ServiceComponent16() override;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSTOI16_HPP
