#ifndef SERVICEIMPL_HPP
#define SERVICEIMPL_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace sample {
  class ServiceComponent12 : public test::Interface1 {
  public:
    ServiceComponent12() = default;
    ~ServiceComponent12() override;
    std::string Description() override;
  };
}

#endif // _SERVICE_IMPL_HPP_
