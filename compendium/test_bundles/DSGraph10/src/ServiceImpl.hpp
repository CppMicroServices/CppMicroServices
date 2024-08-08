#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace graph
{
    class DSGraph10Impl : public test::DSGraph10
    {
      public:
        DSGraph10Impl(std::shared_ptr<test::DSGraph07> const);
        ~DSGraph10Impl() override;
        std::string Description() override;

      private:
        std::shared_ptr<test::DSGraph07> graph02;
    };
} // namespace graph

#endif // _SERVICE_IMPL_HPP_
