#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace graph
{
    class DSGraph06Impl : public test::DSGraph06
    {
      public:
        DSGraph06Impl(std::shared_ptr<test::DSGraph07> const&);
        ~DSGraph06Impl() override;
        std::string Description() override;

      private:
        std::shared_ptr<test::DSGraph07> graph07;
    };
} // namespace graph

#endif // _SERVICE_IMPL_HPP_
