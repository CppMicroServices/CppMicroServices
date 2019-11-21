#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace sample {
  class ServiceComponent14 : public test::Interface1 {
  public:
    ServiceComponent14() = default;
    ~ServiceComponent14() override;
    std::string Description() override;
  };
}

#endif // _SERVICE_IMPL_HPP_
