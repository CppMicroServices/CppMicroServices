#include "ServiceImpl.hpp"

namespace test
{
    globalService1::globalService1() = default;
    globalService1::~globalService1() = default;
    std::string
    globalService1::Description()
    {
        return "globalService1";
    }

    globalService2::globalService2(std::shared_ptr<test::GlobalNS1> const& g1) : graph01(g1) {}

    globalService2::~globalService2() = default;
    std::string
    globalService2::Description()
    {
        return "globalService2";
    }

} // namespace test
