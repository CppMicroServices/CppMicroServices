#ifndef SERVICE_IMPL_DSA_HPP
#define SERVICE_IMPL_DSA_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace sample
{
    class ServiceComponent final : public test::Interface1
    {
      public:
        ServiceComponent(std::shared_ptr<test::Interface1>);
        ~ServiceComponent() override = default;
        std::string Description() override;

      private:
        std::shared_ptr<test::Interface1> m_interface1;
    };
} // namespace sample

#endif // SERVICE_IMPL_DSA_HPP
