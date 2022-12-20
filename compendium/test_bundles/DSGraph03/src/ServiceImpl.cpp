#include "ServiceImpl.hpp"

namespace graph
{
    DSGraph03Impl::DSGraph03Impl(std::shared_ptr<test::DSGraph06> const& g06,
                                 std::shared_ptr<test::DSGraph07> const& g07)
        : graph06(g06)
        , graph07(g07)
    {
    }

    DSGraph03Impl::~DSGraph03Impl() = default;

    std::string
    DSGraph03Impl::Description()
    {
        return STRINGIZE(US_BUNDLE_NAME);
    }
} // namespace graph
