#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

std::string ServiceComponent8::Description()
{
  std::string result(STRINGIZE(US_BUNDLE_NAME));
  return result;
}

} // namespaces
