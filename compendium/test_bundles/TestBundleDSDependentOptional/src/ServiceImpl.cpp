#include "ServiceImpl.hpp"

namespace dependent
{
    TestBundleDSDependentOptionalImpl::TestBundleDSDependentOptionalImpl(
        std::shared_ptr<test::TestBundleDSUpstreamDependency> const& c)
        : test::TestBundleDSDependent()
        , ref(c)
    {
    }

    TestBundleDSDependentOptionalImpl::~TestBundleDSDependentOptionalImpl() = default;
} // namespace dependent
