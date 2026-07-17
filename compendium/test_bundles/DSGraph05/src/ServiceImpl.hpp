#ifndef SERVICE_IMPL_DSGRAPH05_HPP
#define SERVICE_IMPL_DSGRAPH05_HPP

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

#endif // SERVICE_IMPL_DSGRAPH05_HPP
