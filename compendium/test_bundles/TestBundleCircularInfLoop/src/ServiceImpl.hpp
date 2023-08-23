#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentInfLoop1 : public test::DSGraph01
    {
      public:
        ServiceComponentInfLoop1(std::shared_ptr<test::DSGraph02> const& /*ref*/) {}

        std::string
        Description()
        {
            return "";
        };
        ~ServiceComponentInfLoop1() = default;
    };

    class ServiceComponentInfLoop2 : public test::DSGraph02
    {
      public:
        ServiceComponentInfLoop2(std::shared_ptr<test::DSGraph02> const& /*ref*/) {}

        std::string
        Description()
        {
            return "";
        };
        ~ServiceComponentInfLoop2() = default;
    };

} // namespace sample

#endif // _SERVICE_IMPL_HPP_
