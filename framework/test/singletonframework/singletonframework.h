
#include "cppmicroservices/GlobalConfig.h"

#include <cppmicroservices/Framework.h>
#include <memory>

namespace singleton { namespace testing {
  /// Returned a started Framework.
  std::shared_ptr<cppmicroservices::Framework> US_ABI_EXPORT getFramework();
}
}
