#ifndef SERVICEIMPL_HPP
#define SERVICEIMPL_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class ServiceComponent1201 : public test::Interface1
    {
      public:
        ServiceComponent1201() = default;
        ~ServiceComponent1201() override;
        std::string Description() override;
    };
} // namespace sample

#endif // _SERVICE_IMPL_HPP_
