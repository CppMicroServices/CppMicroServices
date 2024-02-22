#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace graph
{
    class DSGraph04Impl : public test::DSGraph04
    {
      public:
        DSGraph04Impl(std::shared_ptr<test::DSGraph05> const&);
        ~DSGraph04Impl() override;
        std::string Description() override;

      private:
        std::shared_ptr<test::DSGraph05> graph05;
    };
} // namespace graph

#endif // _SERVICE_IMPL_HPP_
