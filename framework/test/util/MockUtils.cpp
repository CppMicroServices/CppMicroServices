/*=============================================================================

Library: CppMicroServices

Copyright (c) The CppMicroServices developers. See the COPYRIGHT
file at the top-level directory of this distribution and at
https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=============================================================================*/

#include "MockUtils.h"

using namespace cppmicroservices;
using namespace cppmicroservices::detail;

namespace cppmicroservices
{

    MockedEnvironment::MockedEnvironment(MockBundleStorageMemory* bsm)
        : framework(FrameworkFactory().NewFramework())
    {
        framework.c->storage = std::unique_ptr<MockBundleStorageMemory>(bsm);

        coreBundleContext = framework.c.get();
        delete coreBundleContext->bundleRegistry;
        bundleRegistry = coreBundleContext->bundleRegistry = new MockBundleRegistry(coreBundleContext);
        bundlePrivate = framework.d.get();
        bundleContext = framework.GetBundleContext();
        bundleContextPrivate = bundleContext.d.get();
    }

    std::vector<Bundle>
    MockedEnvironment::Install(
        std::string& location,
        cppmicroservices::AnyMap& bundleManifest,
        MockBundleResourceContainer* resCont
    )
    {
        return bundleRegistry->Install1(location, bundleManifest, resCont);
    }

} // namespace cppmicroservices
