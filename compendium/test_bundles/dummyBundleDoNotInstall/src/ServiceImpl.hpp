#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace graph
{
    class dummyBundleDoNotInstall : public test::DSGraph01
    {
      public:
        dummyBundleDoNotInstall() = default;
        ~dummyBundleDoNotInstall() = default;
        std::string Description() override;
    };
} // namespace graph

#endif // _SERVICE_IMPL_HPP_
