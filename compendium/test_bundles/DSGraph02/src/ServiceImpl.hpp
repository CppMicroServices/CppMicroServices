#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace graph
{
    class DSGraph02Impl : public test::DSGraph02
    {
      public:
        DSGraph02Impl(std::shared_ptr<test::DSGraph04> const&, std::shared_ptr<test::DSGraph05> const&);
        ~DSGraph02Impl() override;
        std::string Description() override;

      private:
        std::shared_ptr<test::DSGraph04> graph04;
        std::shared_ptr<test::DSGraph05> graph05;
    };
} // namespace graph

#endif // _SERVICE_IMPL_HPP_
