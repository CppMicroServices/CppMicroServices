#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "TestInterfaces/Interfaces.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
  class ServiceComponent18 final : public test::Interface3
    {
    public:
      ServiceComponent18():constructorHit{false}{}

      ServiceComponent18(std::shared_ptr<test::Interface1> interface1) : constructorHit{false}
      {
          if (nullptr != interface1)
          {
              constructorHit = true;
          }
      } 

      bool isDependencyInjected() override;
      ~ServiceComponent18() = default;

    private:
        bool constructorHit;
    };
}

#endif // _SERVICE_IMPL_HPP_
