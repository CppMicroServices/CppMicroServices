#include "ServiceImpl.hpp"

namespace graph
{
    DSGraph01Impl::DSGraph01Impl(const std::shared_ptr<test::DSGraph02>& g02
                                 , const std::shared_ptr<test::DSGraph03>& g03)
      : test::DSGraph01()
      , graph02(g02)
      , graph03(g03)
    {
    }
    
    DSGraph01Impl::~DSGraph01Impl() = default;
    
    std::string DSGraph01Impl::Description()
    {
        return STRINGIZE(US_BUNDLE_NAME);
    }
}
