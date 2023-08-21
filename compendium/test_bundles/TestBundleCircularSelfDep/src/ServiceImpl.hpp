#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentSelfDep1 : public test::DSGraph01
    {
      public:
        ServiceComponentSelfDep1(std::shared_ptr<test::DSGraph02> const& /*ref*/) {}

        std::string
        Description()
        {
            return "";
        };
        ~ServiceComponentSelfDep1() = default;
    };

    class ServiceComponentSelfDep2 : public test::DSGraph02
    {
      public:
        ServiceComponentSelfDep2(std::shared_ptr<test::DSGraph02> const& /*ref*/) {}

        std::string
        Description()
        {
            return "";
        };
        ~ServiceComponentSelfDep2() = default;
    };

    class ServiceComponentSelfDep3 : public test::DSGraph02
    {
      public:
        ServiceComponentSelfDep3(std::shared_ptr<test::DSGraph01> const& /*ref*/) {}

        std::string
        Description()
        {
            return "";
        };
        ~ServiceComponentSelfDep3() = default;

      private:
    };

} // namespace sample

#endif // _SERVICE_IMPL_HPP_
