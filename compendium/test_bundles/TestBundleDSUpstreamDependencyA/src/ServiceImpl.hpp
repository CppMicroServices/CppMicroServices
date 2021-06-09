#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"

namespace dependent {
class TestBundleDSUpstreamDependencyImpl
  : public test::TestBundleDSUpstreamDependency
{
public:
  TestBundleDSUpstreamDependencyImpl();
  ~TestBundleDSUpstreamDependencyImpl() override;
};
}

#endif // _SERVICE_IMPL_HPP_
