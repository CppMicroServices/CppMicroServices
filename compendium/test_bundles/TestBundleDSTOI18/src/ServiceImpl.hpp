#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "TestInterfaces/Interfaces.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
  class ServiceComponent18 : public test::Interface3
    {
    public:
      bool constructorHit = false;
     
      ServiceComponent18(std::shared_ptr<test::Interface1> interface1)
      {
		  if(nullptr != interface1)
              constructorHit = true;
      } 

      bool isDependencyInjected() override;
      void Activate(const std::shared_ptr<ComponentContext>&);
      void Deactivate(const std::shared_ptr<ComponentContext>&);
      ~ServiceComponent18() = default;

    private:
    };
}

#endif // _SERVICE_IMPL_HPP_
