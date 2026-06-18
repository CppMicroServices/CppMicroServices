#ifndef SERVICE_IMPL_DSGRAPH07_HPP
#define SERVICE_IMPL_DSGRAPH07_HPP

#include "TestInterfaces/Interfaces.hpp"

namespace graph
{
    class DSGraph07Impl : public test::DSGraph07
    {
      public:
        DSGraph07Impl() = default;
        ~DSGraph07Impl() override;
        std::string Description() override;
    };
} // namespace graph

#endif // SERVICE_IMPL_DSGRAPH07_HPP
