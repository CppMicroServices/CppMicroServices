#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class ServiceComponent16 : public test::Interface1, public test::Interface2
    {
    public:
        std::string Description() override;
        std::string ExtendedDescription() override;
        ~ServiceComponent16() override;
    };
}

#endif // _SERVICE_IMPL_HPP_
