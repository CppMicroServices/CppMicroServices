#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentComplexCircular12
        : public test::DSGraph01
        , public test::DSGraph02
    {
      public:
        ServiceComponentComplexCircular12(std::shared_ptr<test::DSGraph04> const& /*ref*/,
                                          std::shared_ptr<test::DSGraph03> const& /*ref*/)
        {
        }

        std::string Description() {return "";};
        ~ServiceComponentComplexCircular12() = default;
    };

    class ServiceComponentComplexCircular3 : public test::DSGraph03
    {
      public:
        ServiceComponentComplexCircular3(std::shared_ptr<test::DSGraph05> const& /*ref*/) {}

        std::string Description() {return "";};
        ~ServiceComponentComplexCircular3() = default;
    };

    class ServiceComponentComplexCircular4 : public test::DSGraph04
    {
      public:
        ServiceComponentComplexCircular4(std::shared_ptr<test::DSGraph07> const& /*ref*/) {}

        std::string Description() {return "";};
        ~ServiceComponentComplexCircular4() = default;

      private:
    };

    class ServiceComponentComplexCircular56
        : public test::DSGraph05
        , public test::DSGraph06
    {
      public:
        ServiceComponentComplexCircular56(std::shared_ptr<test::DSGraph01> const& /*ref*/) {}

        std::string Description() {return "";};
        ~ServiceComponentComplexCircular56() = default;
    };

    class ServiceComponentComplexCircular7 : public test::DSGraph07
    {
      public:
        ServiceComponentComplexCircular7() {}

        std::string Description() {return "";};
        ~ServiceComponentComplexCircular7() = default;
    };
} // namespace sample

#endif // _SERVICE_IMPL_HPP_
