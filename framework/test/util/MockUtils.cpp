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
using ::testing::_;
using ::testing::Eq;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::AnyOf;

namespace cppmicroservices
{

    MockedEnvironment::MockedEnvironment(bool expectFrameworkStart)
        : framework(FrameworkFactory().NewFramework())
    {
        bundleStorage = new MockBundleStorageMemory();
        framework.c->storage = std::unique_ptr<MockBundleStorageMemory>(bundleStorage);

        coreBundleContext = framework.c.get();
        delete coreBundleContext->bundleRegistry;
        bundleRegistry = coreBundleContext->bundleRegistry = new MockBundleRegistry(coreBundleContext);
        bundlePrivate = framework.d.get();
        bundleContext = framework.GetBundleContext();
        bundleContextPrivate = bundleContext.d.get();

        // Mocked framework bundle
        if (expectFrameworkStart) {
            EXPECT_CALL(*bundleStorage, CreateAndInsertArchive(_, AnyOf(Eq("system_bundle"), Eq("main")), _))
                .Times(AtLeast(1))
                .WillRepeatedly(Return(std::make_shared<MockBundleArchive>(
                    bundleStorage,
                    std::make_shared<MockBundleResourceContainer>(),
                    "MockMain", "main", 0,
                    AnyMap({
                        { "bundle.activator", Any(true) },
                        { "bundle.symbolic_name", Any(std::string("FrameworkBundle")) }
                    })
                )));
            EXPECT_CALL(*bundleStorage, Close()).Times(AtLeast(1));
        }
    }

    std::vector<Bundle>
    MockedEnvironment::Install(
        std::string& location,
        cppmicroservices::AnyMap& bundleManifest,
        std::shared_ptr<MockBundleResourceContainer> const& resCont
    )
    {
        auto bundles = bundleRegistry->Install1(location, bundleManifest, resCont);

        for (auto& b : bundles)
        {
            auto priv = GetPrivate(b);
            priv->lib = std::make_unique<MockSharedLibrary>();
        }

        return bundles;
    }

    template<typename T>
    BundleActivator* createActivator()
    {
        return new T();
    }
    void destroyActivator(BundleActivator* bundleActivator)
    {
        delete bundleActivator;
    }

} // namespace cppmicroservices
