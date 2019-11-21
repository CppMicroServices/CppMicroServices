#include "ServiceImpl.hpp"

namespace sample {
  ServiceComponent::~ServiceComponent()
  {
  }

  std::string ServiceComponent::Description()
  {
    return STRINGIZE(US_BUNDLE_NAME);
  }
}
