#include "ServiceImpl.hpp"

namespace dependent {
TestBundleDSDependentImpl::TestBundleDSDependentImpl(
  const std::shared_ptr<test::TestBundleDSUpstreamDependency>& c)
  : test::TestBundleDSDependent()
  , ref(c)
{
}

TestBundleDSDependentImpl::~TestBundleDSDependentImpl() = default;
}
