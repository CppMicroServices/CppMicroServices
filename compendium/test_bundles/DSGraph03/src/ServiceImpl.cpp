#include "ServiceImpl.hpp"

namespace graph
{
    DSGraph03Impl::DSGraph03Impl(const std::shared_ptr<test::DSGraph06>& g06,
                                 const std::shared_ptr<test::DSGraph07>& g07)
      : graph06(g06)
      , graph07(g07)
    {
    }
    
    DSGraph03Impl::~DSGraph03Impl() = default;
    
    std::string DSGraph03Impl::Description()
    {
        return STRINGIZE(US_BUNDLE_NAME);
    }
}
