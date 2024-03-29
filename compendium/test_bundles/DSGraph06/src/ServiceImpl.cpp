#include "ServiceImpl.hpp"

namespace graph
{
    DSGraph06Impl::~DSGraph06Impl() = default;

    DSGraph06Impl::DSGraph06Impl(std::shared_ptr<test::DSGraph07> const& g07) : graph07(g07) {}
    std::string
    DSGraph06Impl::Description()
    {
        return STRINGIZE(US_BUNDLE_NAME);
    }
} // namespace graph
