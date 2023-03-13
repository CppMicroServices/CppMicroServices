#include "ServiceImpl.hpp"

#include <stdexcept>

namespace dependent
{
    TestBundleDSUpstreamDependencyImpl::TestBundleDSUpstreamDependencyImpl()
    {
        throw std::runtime_error("Failed to create TestBundleDSUpstreamDepdencyImpl");
    }

    TestBundleDSUpstreamDependencyImpl::~TestBundleDSUpstreamDependencyImpl() = default;
} // namespace dependent
