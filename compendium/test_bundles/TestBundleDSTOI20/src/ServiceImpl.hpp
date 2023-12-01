#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class ServiceComponent20 : public test::Interface1
    {
      public:
        ServiceComponent20() = default;
        ~ServiceComponent20() override;
        std::string Description() override;
    };
} // namespace sample

#endif // _SERVICE_IMPL_HPP_
