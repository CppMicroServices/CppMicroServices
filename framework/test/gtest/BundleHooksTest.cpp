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
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/BundleEventHook.h"
#include "cppmicroservices/BundleFindHook.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/GetBundleContext.h"

#include "TestUtils.h"
#include "TestingConfig.h"
#include "gtest/gtest.h"

US_MSVC_PUSH_DISABLE_WARNING(4996)

// conflicts with FrameworkEvent::GetMessage
#undef GetMessage

using namespace cppmicroservices;

class TestBundleListener
{
  public:
    void
    BundleChanged(BundleEvent const& bundleEvent)
    {
        this->events.push_back(bundleEvent);
    }

    std::vector<BundleEvent> events;
};

class TestFrameworkListener
{
  public:
    void
    Event(FrameworkEvent const& frameworkEvent)
    {
        events.push_back(frameworkEvent);
    }

    std::vector<FrameworkEvent> events;
};

class TestBundleFindHook : public BundleFindHook
{
  public:
    void
    Find(BundleContext const& /*context*/, ShrinkableVector<Bundle>& bundles)
    {
        for (auto i = bundles.begin(); i != bundles.end();)
        {
            if (i->GetSymbolicName() == "TestBundleA")
            {
                i = bundles.erase(i);
            }
            else
            {
                ++i;
            }
        }
    }
};

class TestBundleFindHookFailure : public BundleFindHook
{
  public:
    void
    Find(BundleContext const&, ShrinkableVector<Bundle>&)
    {
        throw std::runtime_error("TestBundleFindHookFailure Event exception");
    }
};

class TestBundleEventHook : public BundleEventHook
{
  public:
    void
    Event(BundleEvent const& event, ShrinkableVector<BundleContext>& contexts)
    {
        if (event.GetType() == BundleEvent::BUNDLE_STARTING || event.GetType() == BundleEvent::BUNDLE_STOPPING)
        {
            contexts
                .clear(); // erase(std::remove(contexts.begin(), contexts.end(), GetBundleContext()), contexts.end());
        }
    }
};

class TestBundleEventHookFailure : public BundleEventHook
{
  public:
    void
    Event(BundleEvent const&, ShrinkableVector<BundleContext>&)
    {
        throw std::runtime_error("TestBundleEventHookFailure Event exception");
    }
};

class BundleHooksTest : public ::testing::Test
{
  protected:
    Framework framework;

  public:
    BundleHooksTest() : framework(FrameworkFactory().NewFramework()) {};

    ~BundleHooksTest() override = default;

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
};

TEST_F(BundleHooksTest, TestFindHookBasic)
{
    auto bundleA = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    ASSERT_TRUE(bundleA);

    ASSERT_EQ(bundleA.GetSymbolicName(), "TestBundleA");

    bundleA.Start();

    // Test if started correctly
    ASSERT_TRUE(bundleA.GetState() & Bundle::STATE_ACTIVE);

    // Test for valid bundle id
    long bundleAId = bundleA.GetBundleId();
    EXPECT_GT(bundleAId, 0);

    // Test for non-filtered GetBundle(long) result
    ASSERT_TRUE(framework.GetBundleContext().GetBundle(bundleAId));
    ASSERT_TRUE(bundleA.GetBundleContext().GetBundle(bundleAId));

    auto findHookReg
        = framework.GetBundleContext().RegisterService<BundleFindHook>(std::make_shared<TestBundleFindHook>());

    // Test for filtered GetBundle(long) result
    ASSERT_TRUE(framework.GetBundleContext().GetBundle(bundleAId)); // framework context should NEVER filter
    ASSERT_FALSE(bundleA.GetBundleContext().GetBundle(bundleAId));

    auto bundles = framework.GetBundleContext().GetBundles();
    bool foundBundle = false;
    for (auto const& i : bundles)
    {
        if(i.GetSymbolicName() == "TestBundleA"){
            foundBundle = true;
        }
    }
    ASSERT_TRUE(foundBundle);

    bundles = bundleA.GetBundleContext().GetBundles();
    foundBundle = false;
    for (auto const& i : bundles)
    {
        if(i.GetSymbolicName() == "TestBundleA"){
            foundBundle = true;
        }
    }
    ASSERT_FALSE(foundBundle);

    findHookReg.Unregister();

    bundleA.Stop();
}

TEST_F(BundleHooksTest, TestFindHookBundleInstall)
{
    // install to get diff bundleContext than framework
    auto bundleB = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleB");
    ASSERT_TRUE(bundleB);
    bundleB.Start();

    auto findHookReg
        = framework.GetBundleContext().RegisterService<BundleFindHook>(std::make_shared<TestBundleFindHook>());

    // on first installation, the bundle will not exist and therefore will be installed regardless of hooks
    auto bundleA = cppmicroservices::testing::InstallLib(bundleB.GetBundleContext(), "TestBundleA");
    ASSERT_TRUE(bundleA);
    ASSERT_EQ(bundleA.GetSymbolicName(), "TestBundleA");

    // now that it exists, if installed with non-system bundle it will fail
    bool caught = false;
    try {
        bundleA = cppmicroservices::testing::InstallLib(bundleB.GetBundleContext(), "TestBundleA");
    }
    catch (...) {
        caught = true;
    }
    ASSERT_TRUE(caught);

    // if installed with system, it should still succeed because no filtering
    bundleA = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    ASSERT_TRUE(bundleA);
    ASSERT_EQ(bundleA.GetSymbolicName(), "TestBundleA");

    bundleA.Start();

    findHookReg.Unregister();

    bundleA.Stop();
    bundleB.Stop();
}


TEST_F(BundleHooksTest, TestEventHook)
{
    TestBundleListener bundleListener;
    framework.GetBundleContext().AddBundleListener(&bundleListener, &TestBundleListener::BundleChanged);

    auto bundleA = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    ASSERT_TRUE(bundleA);

    bundleA.Start();

    // Test for received load bundle events
#ifdef US_BUILD_SHARED_LIBS
    ASSERT_EQ(bundleListener.events.size(), 4);
#else
    ASSERT_EQ(bundleListener.events.size(), 3);
#endif

    bundleA.Stop();
    // Test for received unload bundle events
#ifdef US_BUILD_SHARED_LIBS
    ASSERT_EQ(bundleListener.events.size(), 6);
#else
    ASSERT_EQ(bundleListener.events.size(), 5);
#endif

    auto eventHookReg
        = framework.GetBundleContext().RegisterService<BundleEventHook>(std::make_shared<TestBundleEventHook>());

    bundleListener.events.clear();

    bundleA.Start();
    // Test for filtered load bundle events
    ASSERT_EQ(bundleListener.events.size(), 1);
    // Test for BUNDLE_STARTED event
    ASSERT_EQ(bundleListener.events[0].GetType(), BundleEvent::BUNDLE_STARTED);

    bundleA.Stop();
    // Test for filtered unload bundle events
    ASSERT_EQ(bundleListener.events.size(), 2);
    // Test for BUNDLE_STOPPED event
    ASSERT_EQ(bundleListener.events[1].GetType(), BundleEvent::BUNDLE_STOPPED);

    eventHookReg.Unregister();
    framework.GetBundleContext().RemoveBundleListener(&bundleListener, &TestBundleListener::BundleChanged);
}

TEST_F(BundleHooksTest, TestEventHookFailure)
{
    auto bundleA = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    ASSERT_TRUE(bundleA);

    auto eventHookReg
        = framework.GetBundleContext().RegisterService<BundleEventHook>(std::make_shared<TestBundleEventHookFailure>());

    TestFrameworkListener listener;
    auto fwkListenerToken = framework.GetBundleContext().AddFrameworkListener(
        std::bind(&TestFrameworkListener::Event, &listener, std::placeholders::_1));

    bundleA.Start();

    // bundle starting and bundle started events
    ASSERT_EQ(listener.events.size(), 3);

    std::for_each(listener.events.begin(),
                  listener.events.end(),
                  [](FrameworkEvent const& evt)
                  {
                      // Test for the existence of an exception
                      EXPECT_NE(evt.GetThrowable(), nullptr);
                      // Test for the correct framework event type
                      ASSERT_EQ(evt.GetType(), FrameworkEvent::Type::FRAMEWORK_WARNING);
                      std::string msg(evt.GetMessage());
                      // Test for the correct event message
                      ASSERT_NE(std::string::npos, msg.find("Failed to call Bundle EventHook #"));
                  });

    bundleA.Stop();
    eventHookReg.Unregister();
    framework.GetBundleContext().RemoveListener(std::move(fwkListenerToken));
}

TEST_F(BundleHooksTest, TestFindHookFailure)
{
    auto bundleB = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleB");
    ASSERT_TRUE(bundleB);
    bundleB.Start();

    auto eventHookReg
        = framework.GetBundleContext().RegisterService<BundleFindHook>(std::make_shared<TestBundleFindHookFailure>());

    TestFrameworkListener listener;
    auto fwkListenerToken = framework.GetBundleContext().AddFrameworkListener(
        std::bind(&TestFrameworkListener::Event, &listener, std::placeholders::_1));

    auto bundleA = cppmicroservices::testing::InstallLib(bundleB.GetBundleContext(), "TestBundleA");
    ASSERT_TRUE(bundleA);
    bundleA.Start();

    bundleB.GetBundleContext().GetBundle(bundleA.GetBundleId());

    // bundle starting and bundle started events
    // Test for expected number of framework events
#ifdef US_BUILD_SHARED_LIBS
    ASSERT_EQ(listener.events.size(), 1);
#else
    ASSERT_EQ(listener.events.size(), 2);
#endif

    std::for_each(listener.events.begin(),
                  listener.events.end(),
                  [](FrameworkEvent const& evt)
                  {
                      // Test for the existence of an exception
                      ASSERT_NE(evt.GetThrowable(), nullptr);
                      // Test for the correct framework event type
                      ASSERT_EQ(evt.GetType(), FrameworkEvent::Type::FRAMEWORK_WARNING);
                      std::string msg(evt.GetMessage());
                      // Test for the correct event message
                      ASSERT_NE(std::string::npos, msg.find("Failed to call Bundle FindHook  #"));
                  });

    bundleA.Stop();
    bundleB.Stop();
    eventHookReg.Unregister();
    framework.GetBundleContext().RemoveListener(std::move(fwkListenerToken));
}

US_MSVC_POP_WARNING
