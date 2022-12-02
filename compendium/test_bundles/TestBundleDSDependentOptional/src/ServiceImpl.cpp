#include "ServiceImpl.hpp"

namespace dependent {
TestBundleDSDependentOptionalImpl::TestBundleDSDependentOptionalImpl(
  const std::shared_ptr<test::TestBundleDSUpstreamDependency>& c)
  : test::TestBundleDSDependent()
  , ref(c)
{
}

TestBundleDSDependentOptionalImpl::~TestBundleDSDependentOptionalImpl() =
  default;
}
