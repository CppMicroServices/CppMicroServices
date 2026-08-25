#ifndef SERVICE_IMPL_DSGRAPH03_HPP
#define SERVICE_IMPL_DSGRAPH03_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace graph
{
    class DSGraph03Impl : public test::DSGraph03
    {
      public:
        DSGraph03Impl(std::shared_ptr<test::DSGraph06> const&, std::shared_ptr<test::DSGraph07> const&);
        ~DSGraph03Impl() override;
        std::string Description() override;

      private:
        std::shared_ptr<test::DSGraph06> graph06;
        std::shared_ptr<test::DSGraph07> graph07;
    };
} // namespace graph

#endif // SERVICE_IMPL_DSGRAPH03_HPP
