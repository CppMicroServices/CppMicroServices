
#include "cppmicroservices/GlobalConfig.h"

#include <cppmicroservices/Framework.h>
#include <memory>

namespace singleton { namespace testing {
  std::shared_ptr<cppmicroservices::Framework> US_ABI_EXPORT getFramework();
}
}
