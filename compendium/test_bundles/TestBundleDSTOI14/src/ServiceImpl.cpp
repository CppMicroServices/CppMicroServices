#include "ServiceImpl.hpp"

namespace sample {
  ServiceComponent14::~ServiceComponent14()
  {
  }

  std::string ServiceComponent14::Description()
  {
    return STRINGIZE(US_BUNDLE_NAME);
  }
}
