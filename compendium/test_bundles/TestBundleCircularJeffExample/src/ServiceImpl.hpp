#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentJeffExample1 : public test::DSGraph01
    {
      public:
        ServiceComponentJeffExample1(std::shared_ptr<test::DSGraph02> const& /*ref*/) {}

        std::string
        Description()
        {
            return "";
        };
        ~ServiceComponentJeffExample1() = default;
    };

    class ServiceComponentJeffExample2 : public test::DSGraph02
    {
      public:
        ServiceComponentJeffExample2() {}

        std::string
        Description()
        {
            return "";
        };
        ~ServiceComponentJeffExample2() = default;

        void Bindref(std::shared_ptr<test::DSGraph03> const&);
        void Unbindref(std::shared_ptr<test::DSGraph03> const&);
    };

    class ServiceComponentJeffExample3 : public test::DSGraph03
    {
      public:
        ServiceComponentJeffExample3(std::shared_ptr<test::DSGraph01> const& /*ref*/) {}

        std::string
        Description()
        {
            return "";
        };
        ~ServiceComponentJeffExample3() = default;

      private:
    };

} // namespace sample

#endif // _SERVICE_IMPL_HPP_
