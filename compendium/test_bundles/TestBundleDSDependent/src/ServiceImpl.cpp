#include "ServiceImpl.hpp"

namespace dependent
{
    TestBundleDSDependentImpl::TestBundleDSDependentImpl(std::shared_ptr<test::TestBundleDSUpstreamDependency> const& c)
        : test::TestBundleDSDependent()
        , ref(c)
    {
    }

    TestBundleDSDependentImpl::~TestBundleDSDependentImpl() = default;
} // namespace dependent
