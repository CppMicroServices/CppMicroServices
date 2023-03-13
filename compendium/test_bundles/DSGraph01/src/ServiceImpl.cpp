#include "ServiceImpl.hpp"

namespace graph
{
    DSGraph01Impl::DSGraph01Impl(std::shared_ptr<test::DSGraph02> const& g02,
                                 std::shared_ptr<test::DSGraph03> const& g03)
        : test::DSGraph01()
        , graph02(g02)
        , graph03(g03)
    {
    }

    DSGraph01Impl::~DSGraph01Impl() = default;

    std::string
    DSGraph01Impl::Description()
    {
        return STRINGIZE(US_BUNDLE_NAME);
    }
} // namespace graph
