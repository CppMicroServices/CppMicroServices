#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentBasicCircular12
        : public test::DSGraph01
        , public test::DSGraph02
    {
      public:
        ServiceComponentBasicCircular12(std::shared_ptr<test::DSGraph04> const& /*ref*/,
                                          std::shared_ptr<test::DSGraph03> const& /*ref*/)
        {
        }

        std::string Description() {return "";};
        ~ServiceComponentBasicCircular12() = default;
    };

    class ServiceComponentBasicCircular3 : public test::DSGraph03
    {
      public:
        ServiceComponentBasicCircular3(std::shared_ptr<test::DSGraph05> const& /*ref*/) {}

        std::string Description() {return "";};
        ~ServiceComponentBasicCircular3() = default;
    };

    class ServiceComponentBasicCircular4 : public test::DSGraph04
    {
      public:
        ServiceComponentBasicCircular4(std::shared_ptr<test::DSGraph07> const& /*ref*/) {}

        std::string Description() {return "";};
        ~ServiceComponentBasicCircular4() = default;

      private:
    };

    class ServiceComponentBasicCircular56
        : public test::DSGraph05
        , public test::DSGraph06
    {
      public:
        ServiceComponentBasicCircular56(std::shared_ptr<test::DSGraph01> const& /*ref*/) {}

        std::string Description() {return "";};
        ~ServiceComponentBasicCircular56() = default;
    };

    class ServiceComponentBasicCircular7 : public test::DSGraph07
    {
      public:
        ServiceComponentBasicCircular7() {}

        std::string Description() {return "";};
        ~ServiceComponentBasicCircular7() = default;
    };
} // namespace sample

#endif // _SERVICE_IMPL_HPP_
