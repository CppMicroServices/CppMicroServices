#include "ServiceImpl.hpp"

namespace sample {
  ServiceComponent12::~ServiceComponent12()
  {
  }

  std::string ServiceComponent12::Description()
  {
    return STRINGIZE(US_BUNDLE_NAME);
  }
}
