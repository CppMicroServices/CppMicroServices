#ifndef SERVICE_IMPL_DSGRAPH06_HPP
#define SERVICE_IMPL_DSGRAPH06_HPP

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

#endif // SERVICE_IMPL_DSGRAPH06_HPP
