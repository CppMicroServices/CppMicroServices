#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentOptReq1 : public test::DSGraph01
    {
      public:
        ServiceComponentOptReq1(std::shared_ptr<test::DSGraph02> const& /*ref*/) {}

        std::string
        Description()
        {
            return "";
        };
        ~ServiceComponentOptReq1() = default;
    };

    class ServiceComponentOptReq2 : public test::DSGraph02
    {
      public:
        ServiceComponentOptReq2(std::shared_ptr<test::DSGraph03> const& /*ref*/) {}

        std::string
        Description()
        {
            return "";
        };
        ~ServiceComponentOptReq2() = default;
    };

    class ServiceComponentOptReq3 : public test::DSGraph03
    {
      public:
        ServiceComponentOptReq3(std::shared_ptr<test::DSGraph04> const& /*ref*/, std::shared_ptr<test::DSGraph05> const& /*ref*/) {}

        std::string
        Description()
        {
            return "";
        };
        ~ServiceComponentOptReq3() = default;

      private:
    };

    class ServiceComponentOptReq4 : public test::DSGraph04
    {
      public:
        ServiceComponentOptReq4(std::shared_ptr<test::DSGraph01> const& /*ref*/) {}

        std::string
        Description()
        {
            return "";
        };
        ~ServiceComponentOptReq4() = default;
    };

    class ServiceComponentOptReq5 : public test::DSGraph05
    {
      public:
        ServiceComponentOptReq5(std::shared_ptr<test::DSGraph01> const& /*ref*/) {}

        std::string
        Description()
        {
            return "";
        };
        ~ServiceComponentOptReq5() = default;
    };

} // namespace sample

#endif // _SERVICE_IMPL_HPP_
