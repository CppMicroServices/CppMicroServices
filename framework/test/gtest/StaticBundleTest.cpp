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

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/ServiceEvent.h"

#include "TestUtilBundleListener.h"
#include "TestUtilListenerHelpers.h"
#include "TestUtils.h"
#include "TestingConfig.h"
#include "gtest/gtest.h"

US_MSVC_PUSH_DISABLE_WARNING(4996)

using namespace cppmicroservices;

namespace
{

    // Install and start libTestBundleB and check that it exists and that the service it registers exists,
    // also check that the expected events occur
    void
    frame020a(BundleContext context, TestBundleListener& listener)
    {
        auto bundleB = cppmicroservices::testing::InstallLib(context, "TestBundleB");

        // Test for existing bundle TestBundleB
        ASSERT_TRUE(bundleB);

        auto bundleImportedByB = cppmicroservices::testing::GetBundle("TestBundleImportedByB", context);
        // Test for existing bundle TestBundleImportedByB
        ASSERT_TRUE(bundleImportedByB);

        ASSERT_EQ(bundleB.GetSymbolicName(), "TestBundleB");
        ASSERT_EQ(bundleImportedByB.GetSymbolicName(), "TestBundleImportedByB");

        bundleB.Start();
        bundleImportedByB.Start();
        // Check if libB registered the expected service

        std::vector<ServiceReferenceU> refs = context.GetServiceReferences("cppmicroservices::TestBundleBService");
        // Test that both the service from the shared and imported library are regsitered
        ASSERT_EQ(refs.size(), 2);

        auto o1 = context.GetService(refs.front());
        // Test if first service object found
        ASSERT_TRUE(o1 && !o1->empty());

        auto o2 = context.GetService(refs.back());
        // Test if second service object found
        ASSERT_TRUE(o2 && !o2->empty());

        // check the listeners for events
        std::vector<BundleEvent> pEvts;
#ifdef US_BUILD_SHARED_LIBS
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundleB));
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundleImportedByB));
#endif
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, bundleB));
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundleB));
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, bundleB));
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, bundleImportedByB));
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundleImportedByB));
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, bundleImportedByB));

        std::vector<ServiceEvent> seEvts;
        seEvts.push_back(ServiceEvent(ServiceEvent::SERVICE_REGISTERED, refs.front()));
        seEvts.push_back(ServiceEvent(ServiceEvent::SERVICE_REGISTERED, refs.back()));

        bool relaxed = false;
#ifndef US_BUILD_SHARED_LIBS
        relaxed = true;
#endif
        // Test for unexpected events
        ASSERT_TRUE(listener.CheckListenerEvents(pEvts, seEvts, relaxed));

        // Test if started correctly
        ASSERT_EQ(bundleB.GetState(), Bundle::STATE_ACTIVE);
    }

    // Stop libB and check for correct events
    void
    frame030b(BundleContext context, TestBundleListener& listener)
    {
        auto bundleB = cppmicroservices::testing::GetBundle("TestBundleB", context);
        // Test for non-null bundle
        ASSERT_TRUE(bundleB);

        auto bundleImportedByB = cppmicroservices::testing::GetBundle("TestBundleImportedByB", context);
        // Test for non-null bundle
        ASSERT_TRUE(bundleImportedByB);

        std::vector<ServiceReferenceU> refs = context.GetServiceReferences("cppmicroservices::TestBundleBService");
        // Test for first valid service reference
        ASSERT_TRUE(refs.front());
        // Test for second valid service reference
        ASSERT_TRUE(refs.back());

        EXPECT_NO_THROW(bundleB.Stop()) << "Stop bundle exception";
        // Test for stopped state
        ASSERT_EQ(bundleB.GetState(), Bundle::STATE_RESOLVED);

        EXPECT_NO_THROW(bundleImportedByB.Stop()) << "Stop bundle exception";
        // Test for stopped state
        ASSERT_EQ(bundleImportedByB.GetState(), Bundle::STATE_RESOLVED);

        std::vector<BundleEvent> pEvts;
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, bundleB));
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, bundleB));
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, bundleImportedByB));
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, bundleImportedByB));

        std::vector<ServiceEvent> seEvts;
        seEvts.push_back(ServiceEvent(ServiceEvent::SERVICE_UNREGISTERING, refs.front()));
        seEvts.push_back(ServiceEvent(ServiceEvent::SERVICE_UNREGISTERING, refs.back()));

        bool relaxed = false;
#ifndef US_BUILD_SHARED_LIBS
        relaxed = true;
#endif
        // Test for unexpected events
        ASSERT_TRUE(listener.CheckListenerEvents(pEvts, seEvts, relaxed));
    }

// Uninstall libB and check for correct events
#ifdef US_BUILD_SHARED_LIBS
    void
    frame040c(BundleContext context, TestBundleListener& listener)
    {
        bool relaxed = false;

        auto bundleB = cppmicroservices::testing::GetBundle("TestBundleB", context);
        // Test for non-null bundle
        ASSERT_TRUE(bundleB);

        auto bundleImportedByB = cppmicroservices::testing::GetBundle("TestBundleImportedByB", context);
        // Test for non-null bundle
        ASSERT_TRUE(bundleImportedByB);

        auto const bundleCount = context.GetBundles().size();
        // Test for bundle count > 0
        EXPECT_GT(static_cast<int>(bundleCount), 0);
        bundleB.Uninstall();
        // Test for uninstall of TestBundleB
        ASSERT_EQ(context.GetBundles().size(), bundleCount - 1);

        std::vector<BundleEvent> pEvts;
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_UNRESOLVED, bundleB));
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_UNINSTALLED, bundleB));

        // Test for unexpected events
        ASSERT_TRUE(listener.CheckListenerEvents(pEvts, relaxed));

        // Install the same lib again, we should get TestBundleB again
        auto bundles = context.InstallBundles(bundleImportedByB.GetLocation());

        std::size_t installCount = 2;
        // Test for re-install of TestBundleB
        ASSERT_EQ(bundles.size(), installCount);

        long oldId = bundleB.GetBundleId();
        bundleB = cppmicroservices::testing::GetBundle("TestBundleB", context);
        // Test for non-null bundle
        ASSERT_TRUE(bundleB);
        // Test for new bundle id
        ASSERT_NE(oldId, bundleB.GetBundleId());

        pEvts.clear();
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundleB));
        // Test for unexpected events
        ASSERT_TRUE(listener.CheckListenerEvents(pEvts));

        bundleB.Uninstall();
        bundleImportedByB.Uninstall();
        // Test for uninstall of TestBundleImportedByB
        ASSERT_EQ(context.GetBundles().size(), bundleCount - 2);

        pEvts.clear();
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_UNRESOLVED, bundleB));
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_UNINSTALLED, bundleB));
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_UNRESOLVED, bundleImportedByB));
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_UNINSTALLED, bundleImportedByB));

        // Test for unexpected events
        ASSERT_TRUE(listener.CheckListenerEvents(pEvts, relaxed));
    }
#endif

    TEST(StaticBundleTest, testStaticBundle)
    {
        FrameworkFactory factory;
        FrameworkConfiguration frameworkConfig;
        auto framework = factory.NewFramework(frameworkConfig);
        framework.Start();

        auto context = framework.GetBundleContext();

        { // scope the use of the listener so its destructor is
            // called before we destroy the framework's bundle context.
            // The TestBundleListener needs to remove its listeners while
            // the framework is still active.
            TestBundleListener listener;

            BundleListenerRegistrationHelper<TestBundleListener> ml(context,
                                                                    &listener,
                                                                    &TestBundleListener::BundleChanged);
            ServiceListenerRegistrationHelper<TestBundleListener> sl(context,
                                                                     &listener,
                                                                     &TestBundleListener::ServiceChanged);

            frame020a(context, listener);
            frame030b(context, listener);
#ifdef US_BUILD_SHARED_LIBS
            // bundles in the executable are auto-installed.
            // install and uninstall on embedded bundles is not allowed.
            frame040c(context, listener);
#endif
        }
    }
} // namespace

US_MSVC_POP_WARNING
