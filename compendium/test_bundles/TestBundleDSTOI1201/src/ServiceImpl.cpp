#include "ServiceImpl.hpp"

namespace sample
{
    ServiceComponent1201::~ServiceComponent1201() {}

    std::string
    ServiceComponent1201::Description()
    {
        return STRINGIZE(US_BUNDLE_NAME);
    }
} // namespace sample
