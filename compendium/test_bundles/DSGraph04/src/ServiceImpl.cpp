#include "ServiceImpl.hpp"

namespace graph
{
    DSGraph04Impl::~DSGraph04Impl() = default;
    
    DSGraph04Impl::DSGraph04Impl(const std::shared_ptr<test::DSGraph05>& g05)
      : graph05(g05)
    {
    }
    std::string DSGraph04Impl::Description()
    {
        return STRINGIZE(US_BUNDLE_NAME);
    }
}
