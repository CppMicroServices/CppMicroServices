#ifndef SERVICE_IMPL_DSGRAPH04_HPP
#define SERVICE_IMPL_DSGRAPH04_HPP

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

#endif // SERVICE_IMPL_DSGRAPH04_HPP
