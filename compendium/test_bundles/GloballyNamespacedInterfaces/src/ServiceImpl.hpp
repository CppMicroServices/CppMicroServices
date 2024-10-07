#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace test
{
    class globalService1 : public test::GlobalNS1
    {
      public:
        globalService1();
        ~globalService1() override;
        std::string Description() override;

      private:
    };

    class globalService2 : public test::GlobalNS2
    {
      public:
        globalService2(std::shared_ptr<test::GlobalNS1> const&);
        ~globalService2() override;
        std::string Description() override;

      private:
        std::shared_ptr<test::GlobalNS1> graph01;
    };
} // namespace sample

#endif // _SERVICE_IMPL_HPP_
