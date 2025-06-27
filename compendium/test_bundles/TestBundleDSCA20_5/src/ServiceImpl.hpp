#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>
#include <iostream> 

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentCA20_5 : public test::CAInterface1
    {
      public:
        ServiceComponentCA20_5() : active(false) {}

        void
        Activate(std::shared_ptr<ComponentContext> const&)
        {
          active = true;
        }

        void
        Deactivate(std::shared_ptr<ComponentContext> const&)
        {
          active = false;
        }
        cppmicroservices::AnyMap GetProperties(){
          return cppmicroservices::AnyMap { cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS };
        }

        bool isDependencyInjected() { return active; }

        ~ServiceComponentCA20_5() = default;

      private:
        bool active;
    };
} // namespace sample

#endif // _SERVICE_IMPL_HPP_
