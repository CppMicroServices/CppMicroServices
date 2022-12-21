#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace graph
{
    class DSGraph05Impl : public test::DSGraph05
    {
      public:
        DSGraph05Impl(std::shared_ptr<test::DSGraph06> const&);
        ~DSGraph05Impl() override;
        std::string Description() override;

      private:
        std::shared_ptr<test::DSGraph06> graph06;
    };
} // namespace graph

#endif // _SERVICE_IMPL_HPP_
