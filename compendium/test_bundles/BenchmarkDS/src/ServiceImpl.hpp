#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class DSBenchmarkComponent
      : public test::Interface1
    {
      public:
        DSBenchmarkComponent() = default;
        ~DSBenchmarkComponent() override = default;
        std::string Description() override;
    };
}

#endif // _SERVICE_IMPL_HPP_
