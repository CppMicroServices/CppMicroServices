#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentTwoComp1 : public test::DSGraph01
    {
      public:
        ServiceComponentTwoComp1(std::shared_ptr<test::DSGraph01> const& /*ref*/) {}

        std::string
        Description()
        {
            return "";
        };
        ~ServiceComponentTwoComp1() = default;
    };

    class ServiceComponentTwoComp2 : public test::DSGraph01
    {
      public:
        ServiceComponentTwoComp2(std::shared_ptr<test::DSGraph01> const& /*ref*/) {}

        std::string
        Description()
        {
            return "";
        };
        ~ServiceComponentTwoComp2() = default;
    };

} // namespace sample

#endif // _SERVICE_IMPL_HPP_
