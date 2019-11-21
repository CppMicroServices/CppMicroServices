#include "ServiceImpl.hpp"

namespace graph
{
    DSGraph02Impl::DSGraph02Impl(const std::shared_ptr<test::DSGraph04>& g04,
                                 const std::shared_ptr<test::DSGraph05>& g05)
      : graph04(g04)
      , graph05(g05)
    {
    }
    
    DSGraph02Impl::~DSGraph02Impl() = default;
    
    std::string DSGraph02Impl::Description()
    {
        return STRINGIZE(US_BUNDLE_NAME);
    }
}
