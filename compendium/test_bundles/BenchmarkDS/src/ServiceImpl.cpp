#include "ServiceImpl.hpp"

namespace sample {
  std::string DSBenchmarkComponent::Description()
  {
    return STRINGIZE(US_BUNDLE_NAME);
  }
}
