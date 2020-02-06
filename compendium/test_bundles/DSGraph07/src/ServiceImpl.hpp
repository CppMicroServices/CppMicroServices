#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace graph
{
    class DSGraph07Impl
      : public test::DSGraph07
    {
      public:
        DSGraph07Impl() = default;
        ~DSGraph07Impl() override;
        std::string Description() override;
    };
}

#endif // _SERVICE_IMPL_HPP_
