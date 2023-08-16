#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <mutex>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample
{
    class ServiceComponentTwoCircles1
        : public test::DSGraph01
    {
      public:
        ServiceComponentTwoCircles1(std::shared_ptr<test::DSGraph02> const& /*ref*/)
        {
        }

        std::string Description() {return "";};
        ~ServiceComponentTwoCircles1() = default;
    };

    class ServiceComponentTwoCircles2 : public test::DSGraph02
    {
      public:
        ServiceComponentTwoCircles2(std::shared_ptr<test::DSGraph03> const& /*ref*/) {}

        std::string Description() {return "";};
        ~ServiceComponentTwoCircles2() = default;
    };

    class ServiceComponentTwoCircles3 : public test::DSGraph03
    {
      public:
        ServiceComponentTwoCircles3(std::shared_ptr<test::DSGraph01> const& /*ref*/) {}

        std::string Description() {return "";};
        ~ServiceComponentTwoCircles3() = default;

      private:
    };

    class ServiceComponentTwoCircles4
        : public test::DSGraph04
    {
      public:
        ServiceComponentTwoCircles4(std::shared_ptr<test::DSGraph05> const& /*ref*/) {}

        std::string Description() {return "";};
        ~ServiceComponentTwoCircles4() = default;
    };

    class ServiceComponentTwoCircles5 : public test::DSGraph05
    {
      public:
        ServiceComponentTwoCircles5(std::shared_ptr<test::DSGraph06> const& /*ref*/) {}

        std::string Description() {return "";};
        ~ServiceComponentTwoCircles5() = default;
    };

    class ServiceComponentTwoCircles6 : public test::DSGraph06
    {
      public:
        ServiceComponentTwoCircles6(std::shared_ptr<test::DSGraph04> const& /*ref*/,std::shared_ptr<test::DSGraph07> const& /*ref*/) {}

        std::string Description() {return "";};
        ~ServiceComponentTwoCircles6() = default;
    };

    class ServiceComponentTwoCircles7 : public test::DSGraph07
    {
      public:
        ServiceComponentTwoCircles7() {}

        std::string Description() {return "";};
        ~ServiceComponentTwoCircles7() = default;
    };
} // namespace sample

#endif // _SERVICE_IMPL_HPP_
