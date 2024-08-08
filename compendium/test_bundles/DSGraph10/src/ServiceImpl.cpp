#include "ServiceImpl.hpp"

namespace graph
{

    DSGraph10Impl::DSGraph10Impl(std::shared_ptr<test::DSGraph07> const g02) : test::DSGraph10(), graph02(g02) {}

    DSGraph10Impl::~DSGraph10Impl() = default;

    std::string
    DSGraph10Impl::Description()
    {
        return STRINGIZE(US_BUNDLE_NAME) + std::string(" ") + graph02->Description();
    }
} // namespace graph
