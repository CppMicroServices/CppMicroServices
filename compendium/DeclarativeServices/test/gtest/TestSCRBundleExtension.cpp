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

#include "../../src/SCRAsyncWorkService.hpp"
#include "../../src/SCRBundleExtension.hpp"
#include "../../src/SCRExtensionRegistry.hpp"
#include "../../src/metadata/Util.hpp"
#include "Mocks.hpp"
#include "../TestUtils.hpp"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include <chrono>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>

#define str(s)  #s
#define xstr(s) str(s)

using cppmicroservices::Any;
using cppmicroservices::service::component::ComponentConstants::SERVICE_COMPONENT;

namespace cppmicroservices
{
    namespace scrimpl
    {

        using cppmicroservices::AnyMap;
        // The fixture for testing class SCRBundleExtension.
        class SCRBundleExtensionTest : public ::testing::Test
        {
          protected:
            SCRBundleExtensionTest() : framework(cppmicroservices::FrameworkFactory().NewFramework()) {}
            ~SCRBundleExtensionTest() = default;

            void
            SetUp() override
            {
                framework.Start();
            }

            void
            TearDown() override
            {
                framework.Stop();
                framework.WaitForStop(std::chrono::milliseconds::zero());
            }

            cppmicroservices::Framework&
            GetFramework()
            {
                return framework;
            }

          private:
            cppmicroservices::Framework framework;
        };

        TEST_F(SCRBundleExtensionTest, CtorInvalidArgs)
        {
            auto mockRegistry = std::make_shared<MockComponentRegistry>();
            auto fakeLogger = std::make_shared<FakeLogger>();
            auto logger = std::make_shared<cppmicroservices::scrimpl::SCRLogger>(GetFramework().GetBundleContext());
            auto bundleContext = GetFramework().GetBundleContext();
            auto extRegistry = std::make_shared<SCRExtensionRegistry>(logger);
            auto asyncWorkService
                = std::make_shared<cppmicroservices::scrimpl::SCRAsyncWorkService>(GetFramework().GetBundleContext(),
                                                                                   logger);
            auto notifier
                = std::make_shared<ConfigurationNotifier>(bundleContext, fakeLogger, asyncWorkService, extRegistry);
            EXPECT_THROW(
                {
                    SCRBundleExtension bundleExt(Bundle(),
                                                 mockRegistry,
                                                 fakeLogger,
                                                 notifier);
                },
                std::invalid_argument);
            EXPECT_THROW(
                {
                    SCRBundleExtension bundleExt(GetFramework(),
                                                 nullptr,
                                                 fakeLogger,
                                                 notifier);
                },
                std::invalid_argument);
            EXPECT_THROW(
                {
                    SCRBundleExtension bundleExt(GetFramework(),
                                                 mockRegistry,
                                                 nullptr,
                                                 notifier);
                },
                std::invalid_argument);
            EXPECT_THROW(
                {
                    SCRBundleExtension bundleExt(GetFramework(),
                                                 mockRegistry,
                                                 fakeLogger,
                                                 nullptr);
                },
                std::invalid_argument);
        }
        TEST_F(SCRBundleExtensionTest, InitializeWithInvalidArgs)
        {
            auto mockRegistry = std::make_shared<MockComponentRegistry>();
            auto logger = std::make_shared<cppmicroservices::scrimpl::SCRLogger>(GetFramework().GetBundleContext());
            auto bundleContext = GetFramework().GetBundleContext();
            auto extRegistry = std::make_shared<SCRExtensionRegistry>(logger);
            cppmicroservices::AnyMap headers(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
            auto asyncWorkService
                = std::make_shared<cppmicroservices::scrimpl::SCRAsyncWorkService>(GetFramework().GetBundleContext(),
                                                                                   logger);
            auto notifier
                = std::make_shared<ConfigurationNotifier>(bundleContext, logger, asyncWorkService, extRegistry);
            SCRBundleExtension bundleExt(GetFramework(), mockRegistry, logger, notifier); 
            EXPECT_THROW({ bundleExt.Initialize(headers, asyncWorkService); },
                         std::invalid_argument);
        }
        TEST_F(SCRBundleExtensionTest, CtorWithValidArgs)
        {
            auto bundles = GetFramework().GetBundleContext().GetBundles();
            auto thisBundleItr = std::find_if(bundles.begin(),
                                              bundles.end(),
                                              [](cppmicroservices::Bundle const& bundle)
                                              { return (bundle.GetSymbolicName() == xstr(US_BUNDLE_NAME)); });
            auto thisBundle = thisBundleItr != bundles.end() ? *thisBundleItr : cppmicroservices::Bundle();
            ASSERT_TRUE(static_cast<bool>(thisBundle));
            auto const& scr = ref_any_cast<cppmicroservices::AnyMap>(thisBundle.GetHeaders().at("scr_test_0"));
             auto mockRegistry = std::make_shared<MockComponentRegistry>();
            EXPECT_CALL(*mockRegistry, AddComponentManager(testing::_))
                .Times(2)
                .WillOnce(testing::Throw(std::runtime_error("Failed to add component")))
                .WillOnce(testing::Return(true));
            EXPECT_CALL(*mockRegistry, RemoveComponentManager(testing::_)).Times(1);
            auto fakeLogger = std::make_shared<FakeLogger>();
            auto logger = std::make_shared<cppmicroservices::scrimpl::SCRLogger>(GetFramework().GetBundleContext());
            auto asyncWorkService
                = std::make_shared<cppmicroservices::scrimpl::SCRAsyncWorkService>(GetFramework().GetBundleContext(),
                                                                                   logger);
            auto extRegistry = std::make_shared<SCRExtensionRegistry>(logger);
            auto notifier = std::make_shared<ConfigurationNotifier>(GetFramework().GetBundleContext(),
                                                                    fakeLogger,
                                                                    asyncWorkService,
                                                                    extRegistry); 
            EXPECT_NO_THROW({
                SCRBundleExtension bundleExt = SCRBundleExtension(GetFramework(), mockRegistry, fakeLogger, notifier);
                bundleExt.Initialize(scr, asyncWorkService);
                EXPECT_EQ(bundleExt.managers.size(), 0u);
            });
            EXPECT_NO_THROW({
                SCRBundleExtension bundleExt(GetFramework(), mockRegistry, fakeLogger, notifier);
                bundleExt.Initialize(scr, asyncWorkService);
                EXPECT_EQ(bundleExt.managers.size(), 1u);
            });
        }

        // Simulate a DS bundle is stopped before DS is able to cleanup the data associated
        // with the bundle.
        TEST_F(SCRBundleExtensionTest, DtorNoThrow)
        {
            auto bundle = test::InstallAndStartBundle(GetFramework().GetBundleContext(), "TestBundleDSTOI1");
            ASSERT_TRUE(static_cast<bool>(bundle));
            auto fakeRegistry = std::make_shared<ComponentRegistry>();
            auto fakeLogger = std::make_shared<FakeLogger>();
            auto asyncWorkService
                = std::make_shared<cppmicroservices::scrimpl::SCRAsyncWorkService>(GetFramework().GetBundleContext(),
                                                                                   fakeLogger);
            auto logger = std::make_shared<cppmicroservices::scrimpl::SCRLogger>(GetFramework().GetBundleContext());
            auto extRegistry = std::make_shared<SCRExtensionRegistry>(logger);
            auto notifier = std::make_shared<ConfigurationNotifier>(GetFramework().GetBundleContext(),
                                                                    fakeLogger,
                                                                    asyncWorkService,
                                                                    extRegistry);
 
            EXPECT_NO_THROW({
                SCRBundleExtension bundleExt(bundle,  fakeRegistry, fakeLogger,  notifier);
                // stop the bundle prior to ~SCRBundleExtension being called. ~SCRBundleExtension
                // attempts to access the Bundle object, so make sure accessing an invalid
                // Bundle object doesn't throw.
                bundle.Stop();
            });

            asyncWorkService->StopTracking();
            fakeRegistry->Clear();
        }
    } // namespace scrimpl
} // namespace cppmicroservices
