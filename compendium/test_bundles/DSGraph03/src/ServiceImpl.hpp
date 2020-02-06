#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace graph
{
    class DSGraph03Impl
      : public test::DSGraph03
    {
      public:
        DSGraph03Impl(const std::shared_ptr<test::DSGraph06>&, const std::shared_ptr<test::DSGraph07>&);
        ~DSGraph03Impl() override;
        std::string Description() override;
      private:
        std::shared_ptr<test::DSGraph06> graph06;
        std::shared_ptr<test::DSGraph07> graph07;
    };
}

#endif // _SERVICE_IMPL_HPP_
