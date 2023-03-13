#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace graph
{
    class DSGraph01Impl : public test::DSGraph01
    {
      public:
        DSGraph01Impl(std::shared_ptr<test::DSGraph02> const&, std::shared_ptr<test::DSGraph03> const&);
        ~DSGraph01Impl() override;
        std::string Description() override;

      private:
        std::shared_ptr<test::DSGraph02> graph02;
        std::shared_ptr<test::DSGraph03> graph03;
    };
} // namespace graph

#endif // _SERVICE_IMPL_HPP_
