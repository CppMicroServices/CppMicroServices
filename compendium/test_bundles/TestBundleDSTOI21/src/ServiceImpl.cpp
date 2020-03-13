#include "ServiceImpl.hpp"
#include <iostream>

namespace sample {

std::string ServiceComponent21::Description()
{
  std::string result(STRINGIZE(US_BUNDLE_NAME));
  return result;
}

} // namespaces
