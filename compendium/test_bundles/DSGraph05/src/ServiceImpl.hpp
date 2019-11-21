#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace graph
{
    class DSGraph05Impl
      : public test::DSGraph05
    {
      public:
        DSGraph05Impl(const std::shared_ptr<test::DSGraph06>&);
        ~DSGraph05Impl() override;
        std::string Description() override;
      private:
        std::shared_ptr<test::DSGraph06> graph06;
    };
}

#endif // _SERVICE_IMPL_HPP_
