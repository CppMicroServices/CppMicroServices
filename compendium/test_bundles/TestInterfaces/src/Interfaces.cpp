#include "TestInterfaces/Interfaces.hpp"

namespace test
{
    Interface1::~Interface1() = default;
    Interface2::~Interface2() = default;
    Interface3::~Interface3() = default;

    TestBundleDSDependent::~TestBundleDSDependent() = default;
    TestBundleDSUpstreamDependency::~TestBundleDSUpstreamDependency() = default;

    DSGraph01::~DSGraph01() = default;
    DSGraph02::~DSGraph02() = default;
    DSGraph03::~DSGraph03() = default;
    DSGraph04::~DSGraph04() = default;
    DSGraph05::~DSGraph05() = default;
    DSGraph06::~DSGraph06() = default;
    DSGraph07::~DSGraph07() = default;

    LifeCycleValidation::~LifeCycleValidation() = default;
    CAInterface::~CAInterface() = default;
    CAInterface1::~CAInterface1() = default;
    ServiceAInt::~ServiceAInt() = default;
    ServiceBInt::~ServiceBInt() = default;
    ServiceCInt::~ServiceCInt() = default;
} // namespace test
