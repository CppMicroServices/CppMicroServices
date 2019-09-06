#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace sample {
  class ServiceComponent15 : public test::Interface1 {
  public:
    ServiceComponent15() = default;
    ~ServiceComponent15() override;
    std::string Description() override;
  };
}

#endif // _SERVICE_IMPL_HPP_
