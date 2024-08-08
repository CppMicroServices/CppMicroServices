#include "ServiceImpl.hpp"

namespace graph
{

    DSGraph09Impl::DSGraph09Impl(std::shared_ptr<test::DSGraph07> const&& g02) : test::DSGraph09(), graph02(g02) {}
    // DSGraph09Impl::DSGraph09Impl() : test::DSGraph09(){}

    DSGraph09Impl::~DSGraph09Impl() = default;

    std::string
    DSGraph09Impl::Description()
    {
        return STRINGIZE(US_BUNDLE_NAME) + std::string(" ") + graph02->Description();
    }
} // namespace graph
