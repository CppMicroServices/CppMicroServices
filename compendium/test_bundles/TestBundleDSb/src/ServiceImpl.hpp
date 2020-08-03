#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace sample {
  class ServiceComponent : public test::Interface1 {
  public:
    ServiceComponent() = default;
    ~ServiceComponent() override;
    std::string Description() override;
  };
}

#endif // _SERVICE_IMPL_HPP_
