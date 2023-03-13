#include "ServiceImpl.hpp"

namespace dependent
{
    TestBundleDSDependentNoInjectImpl::TestBundleDSDependentNoInjectImpl() : test::TestBundleDSDependent() {}

    TestBundleDSDependentNoInjectImpl::~TestBundleDSDependentNoInjectImpl() = default;
} // namespace dependent
