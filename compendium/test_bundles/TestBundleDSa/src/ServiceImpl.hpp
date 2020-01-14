#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace sample {
  class ServiceComponent final : public test::Interface1 {
  public:
    ServiceComponent(std::shared_ptr<test::Interface1>);
    ~ServiceComponent() override = default;
    std::string Description() override;
    
  private:
    std::shared_ptr<test::Interface1> m_interface1;  
  };
}

#endif // _SERVICE_IMPL_HPP_
