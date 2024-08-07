#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace graph
{
    class DSGraph08Impl : public test::DSGraph08
    {
      public:
        DSGraph08Impl(std::shared_ptr<cppmicroservices::AnyMap> properties);
        DSGraph08Impl() = default;
        ~DSGraph08Impl() override;
        std::string Description() override;

      private:
        std::shared_ptr<cppmicroservices::AnyMap> props;
    };
} // namespace graph

#endif // _SERVICE_IMPL_HPP_
