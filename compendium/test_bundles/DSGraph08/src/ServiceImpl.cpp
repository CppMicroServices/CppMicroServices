#include "ServiceImpl.hpp"

namespace graph
{
    DSGraph08Impl::~DSGraph08Impl() = default;
    DSGraph08Impl::DSGraph08Impl(std::shared_ptr<cppmicroservices::AnyMap> properties) : props(properties) { return; }

    std::string
    DSGraph08Impl::Description()
    {
        return STRINGIZE(US_BUNDLE_NAME);
    }
} // namespace graph
