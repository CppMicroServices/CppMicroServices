#include "ServiceImpl.hpp"

namespace graph
{
    std::string
    dummyBundleDoNotInstall::Description()
    {
        return STRINGIZE(US_BUNDLE_NAME);
    }
} // namespace graph
