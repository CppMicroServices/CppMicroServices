#include "ServiceImpl.hpp"

namespace sample {
  void ServiceComponent10::Activate(const std::shared_ptr<ComponentContext>&)
  {
    activated = true;
  };
  void ServiceComponent10::Deactivate(const std::shared_ptr<ComponentContext>&)
  {
    deactivated = true;
  };
  ServiceComponent10::~ServiceComponent10() {};
}
