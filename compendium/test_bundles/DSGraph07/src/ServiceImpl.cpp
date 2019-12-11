#include "ServiceImpl.hpp"

namespace graph
{
    DSGraph07Impl::~DSGraph07Impl() = default;
    
    std::string DSGraph07Impl::Description()
    {
        return STRINGIZE(US_BUNDLE_NAME);
    }
}
