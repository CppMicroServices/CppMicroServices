#ifndef SERVICE_IMPL_BENCHMARKDS_HPP
#define SERVICE_IMPL_BENCHMARKDS_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class DSBenchmarkComponent : public test::Interface1
    {
      public:
        DSBenchmarkComponent() = default;
        ~DSBenchmarkComponent() override = default;
        std::string Description() override;
    };
} // namespace sample

#endif // SERVICE_IMPL_BENCHMARKDS_HPP
