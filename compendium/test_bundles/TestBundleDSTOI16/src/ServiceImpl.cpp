#include "ServiceImpl.hpp"

namespace sample
{
    std::string ServiceComponent16::Description()
    {
      return STRINGIZE(US_BUNDLE_NAME);
    }

  std::string ServiceComponent16::ExtendedDescription()
  {
    return "This is a test bundle used to verify service components that implement multiple interfaces";
  }

  ServiceComponent16::~ServiceComponent16() {}
}
