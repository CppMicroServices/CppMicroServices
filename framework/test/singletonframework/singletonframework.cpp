
#include "singletonframework.h"

#include <cppmicroservices/Any.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <memory>

namespace singleton { namespace testing {

std::shared_ptr<cppmicroservices::Framework> getFramework()
{
  static std::shared_ptr<cppmicroservices::Framework> framework = std::make_shared<cppmicroservices::Framework>(cppmicroservices::FrameworkFactory().NewFramework());
  framework->Start();
  return framework;
}

}
}
