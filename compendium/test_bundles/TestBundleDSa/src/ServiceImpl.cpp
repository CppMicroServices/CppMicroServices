#include "ServiceImpl.hpp"

namespace sample {
  ServiceComponent::ServiceComponent(std::shared_ptr<test::Interface1> interface1)
      : m_interface1(std::move(interface1)) { }

  std::string ServiceComponent::Description()
  {
    return m_interface1->Description();
  }
}
