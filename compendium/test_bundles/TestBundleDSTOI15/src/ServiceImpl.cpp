#include "ServiceImpl.hpp"

namespace sample {
  ServiceComponent15::~ServiceComponent15()
  {
  }

  std::string ServiceComponent15::Description()
  {
    return STRINGIZE(US_BUNDLE_NAME);
  }
}
