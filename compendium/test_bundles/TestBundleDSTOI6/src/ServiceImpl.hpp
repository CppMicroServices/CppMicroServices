#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "TestInterfaces/Interfaces.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
  class ServiceComponent6 : public test::Interface2
    {
    public:
      ServiceComponent6() = default;
      std::string ExtendedDescription() override;
      void Activate(const std::shared_ptr<ComponentContext>&);
      void Deactivate(const std::shared_ptr<ComponentContext>&);
      ~ServiceComponent6() = default;

      void Bindfoo(const std::shared_ptr<test::Interface1>&);
      void Unbindfoo(const std::shared_ptr<test::Interface1>&);
    private:
      std::shared_ptr<test::Interface1> foo;
    };
}

#endif // _SERVICE_IMPL_HPP_
