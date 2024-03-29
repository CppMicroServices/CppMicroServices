#include "ServiceImpl.hpp"

namespace graph
{
    DSGraph05Impl::~DSGraph05Impl() = default;

    DSGraph05Impl::DSGraph05Impl(std::shared_ptr<test::DSGraph06> const& g06) : graph06(g06) {}
    std::string
    DSGraph05Impl::Description()
    {
        return STRINGIZE(US_BUNDLE_NAME);
    }
} // namespace graph
