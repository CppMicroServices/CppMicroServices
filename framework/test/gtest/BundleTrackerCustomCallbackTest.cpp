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
#include "cppmicroservices/BundleTracker.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "TestUtils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#ifdef GetObject
#  undef GetObject
#endif

using namespace cppmicroservices;

class BundleTrackerCustomCallbackTest : public ::testing::Test
{
protected:
  Framework framework;
  BundleContext context;
  BundleTracker<>::StateType all_states =
    Bundle::State::STATE_ACTIVE | Bundle::State::STATE_INSTALLED |
    Bundle::State::STATE_RESOLVED | Bundle::State::STATE_STARTING |
    Bundle::State::STATE_STOPPING | Bundle::State::STATE_UNINSTALLED;

public:
  BundleTrackerCustomCallbackTest()
    : framework(FrameworkFactory().NewFramework()){};

  ~BundleTrackerCustomCallbackTest() override = default;

  void SetUp() override
  {
    framework.Start();
    context = framework.GetBundleContext();
  }

  void TearDown() override
  {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }
};

//
// Test custom implementation through a BundleTrackerCustomizer subclass tracking bundles
//
class MockCustomizer : public BundleTrackerCustomizer<>
{
public:
  MOCK_METHOD(std::optional<Bundle>,
              AddingBundle,
              (const Bundle&, const BundleEvent&),
              (override));
  MOCK_METHOD(void,
              ModifiedBundle,
              (const Bundle&, const BundleEvent&, Bundle),
              (override));
  MOCK_METHOD(void,
              RemovedBundle,
              (const Bundle&, const BundleEvent&, Bundle),
              (override));
};

TEST_F(BundleTrackerCustomCallbackTest,
       BundleIsTrackedWhenReturnedByAddingBundleFromCustomizer)
{
  auto customizer = std::make_shared<MockCustomizer>();
  auto bundleTracker =
    std::make_shared<BundleTracker<>>(context, all_states, customizer);

  // Make AddingBundle return null for bundles we aren't testing (main, system_bundle)
  EXPECT_CALL(*customizer, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";

  // bundle: ---> INSTALLED (Add)
  EXPECT_CALL(*customizer, AddingBundle)
    .Times(1)
    .WillOnce(::testing::ReturnArg<0>()); // returns the bundle
  EXPECT_CALL(*customizer, ModifiedBundle).Times(0);
  EXPECT_CALL(*customizer, RemovedBundle).Times(0);
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  EXPECT_EQ(bundle, bundleTracker->GetObject(bundle))
    << "The bundle returned by a customizer's AddingBundle should be tracked";

  // Whatever happens after the bundle is added is out of scope for this test
  EXPECT_CALL(*customizer, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker->Close();
}

TEST_F(BundleTrackerCustomCallbackTest,
       BundleUntrackedIfAddingBundleFromCustomizerReturnsNull)
{
  auto customizer = std::make_shared<MockCustomizer>();
  auto bundleTracker =
    std::make_shared<BundleTracker<>>(context, all_states, customizer);

  // Make AddingBundle return null for all bundles
  EXPECT_CALL(*customizer, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  Bundle ignoredBundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleM");

  EXPECT_EQ(0, bundleTracker->Size())
    << "No bundles should be tracked if AddingBundle always returns null";

  bundleTracker->Close();
}

TEST_F(BundleTrackerCustomCallbackTest,
       CallbacksWorkForCustomizerTrackingBundlesAfterBundleModified)
{
  auto customizer = std::make_shared<MockCustomizer>();
  auto bundleTracker =
    std::make_shared<BundleTracker<>>(context, all_states, customizer);
  EXPECT_CALL(*customizer, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  EXPECT_CALL(*customizer, AddingBundle)
    .Times(1)
    .WillOnce(::testing::ReturnArg<0>());
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  // bundle: INSTALLED ---> RESOLVED (Modify)
  //                   ---> STARTING (Modify)
  //                   ---> ACTIVE (Modify)
  EXPECT_CALL(*customizer, AddingBundle).Times(0);
  EXPECT_CALL(*customizer, ModifiedBundle).Times(3);
  EXPECT_CALL(*customizer, RemovedBundle).Times(0);
  bundle.Start();

  // What happens after the bundle is modified is out of scope for this test
  EXPECT_CALL(*customizer, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker->Close();
}

TEST_F(BundleTrackerCustomCallbackTest,
       CallbacksWorkForCustomizerTrackingBundlesAfterBundleRemoved)
{
  auto customizer = std::make_shared<MockCustomizer>();
  auto stateMask = Bundle::State::STATE_INSTALLED;
  auto bundleTracker =
    std::make_shared<BundleTracker<>>(context, stateMask, customizer);
  EXPECT_CALL(*customizer, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";

  EXPECT_CALL(*customizer, AddingBundle)
    .WillRepeatedly(::testing::ReturnArg<0>());
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  Bundle bundle2 = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleM");

  // bundle: INSTALLED ---> UNINSTALLED (Remove)
  EXPECT_CALL(*customizer, AddingBundle).Times(0);
  EXPECT_CALL(*customizer, ModifiedBundle).Times(0);
  EXPECT_CALL(*customizer, RemovedBundle).Times(1);
  bundle.Uninstall();

  // When the BundleTracker is closed, RemovedBundle should be called
  // for the tracked bundle bundle2
  EXPECT_CALL(*customizer, RemovedBundle).Times(1);
  bundleTracker->Close();
}

//
// Test custom implementation through a BundleTrackerCustomizer subclass tracking custom objects
//
class MockCustomizerWithObject : public BundleTrackerCustomizer<int>
{
public:
  MOCK_METHOD(std::optional<int>,
              AddingBundle,
              (const Bundle&, const BundleEvent&),
              (override));
  MOCK_METHOD(void,
              ModifiedBundle,
              (const Bundle&, const BundleEvent&, int),
              (override));
  MOCK_METHOD(void,
              RemovedBundle,
              (const Bundle&, const BundleEvent&, int),
              (override));
};

TEST_F(BundleTrackerCustomCallbackTest,
       ObjectIsTrackedWhenReturnedByAddingBundleFromCustomizer)
{
  auto customizer = std::make_shared<MockCustomizerWithObject>();
  auto bundleTracker =
    std::make_shared<BundleTracker<int>>(context, all_states, customizer);

  // Make AddingBundle return null for bundles we aren't testing (main, system_bundle)
  EXPECT_CALL(*customizer, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";

  // bundle: ---> INSTALLED (Add)
  EXPECT_CALL(*customizer, AddingBundle)
    .Times(1)
    .WillOnce(::testing::Return(1)); // returns the object 1
  EXPECT_CALL(*customizer, ModifiedBundle).Times(0);
  EXPECT_CALL(*customizer, RemovedBundle).Times(0);
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  EXPECT_EQ(1, bundleTracker->GetObject(bundle))
    << "The object returned by a customizer's AddingBundle should be tracked";

  // Whatever happens after the bundle is added is out of scope for this test
  EXPECT_CALL(*customizer, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker->Close();
}

class MockBundleTracker : public BundleTracker<>
{
public:
  MockBundleTracker(const BundleContext& context, StateType stateMask)
    : BundleTracker(context, stateMask)
  {}

  MOCK_METHOD(std::optional<Bundle>,
              AddingBundle,
              (const Bundle& bundle, const BundleEvent&),
              (override));
  MOCK_METHOD(void,
              ModifiedBundle,
              (const Bundle&, const BundleEvent&, Bundle),
              (override));
  MOCK_METHOD(void,
              RemovedBundle,
              (const Bundle&, const BundleEvent&, Bundle),
              (override));
};

TEST_F(BundleTrackerCustomCallbackTest,
       BundleIsTrackedWhenReturnedByAddingBundleFromOverride)
{
  auto bundleTracker = std::make_shared<MockBundleTracker>(context, all_states);
  // Make AddingBundle return null for bundles we aren't testing (main, system_bundle)
  EXPECT_CALL(*bundleTracker, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";

  // bundles: ---> INSTALLED (Add)
  EXPECT_CALL(*bundleTracker, AddingBundle)
    .Times(1)
    .WillOnce(::testing::ReturnArg<0>()); // returns the bundle
  EXPECT_CALL(*bundleTracker, ModifiedBundle).Times(0);
  EXPECT_CALL(*bundleTracker, RemovedBundle).Times(0);
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  EXPECT_EQ(bundle, bundleTracker->GetObject(bundle))
    << "The bundle returned by AddingBundle from a BundleTracker subclass "
       "should be tracked";

  // Whatever happens after the bundle is added is out of scope for this test
  EXPECT_CALL(*bundleTracker, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker->Close();
}

TEST_F(BundleTrackerCustomCallbackTest,
       BundleUntrackedIfAddingBundleFromOverrideReturnsNull)
{
  auto bundleTracker = std::make_shared<MockBundleTracker>(context, all_states);

  // Make AddingBundle return null for all bundles
  EXPECT_CALL(*bundleTracker, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  Bundle ignoredBundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleM");

  EXPECT_EQ(0, bundleTracker->Size())
    << "No bundles should be tracked if AddingBundle always returns null";

  bundleTracker->Close();
}

TEST_F(BundleTrackerCustomCallbackTest,
       CallbacksWorkForOverridenTrackerTrackingBundlesAfterBundleModified)
{
  auto bundleTracker = std::make_shared<MockBundleTracker>(context, all_states);
  EXPECT_CALL(*bundleTracker, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  EXPECT_CALL(*bundleTracker, AddingBundle)
    .Times(1)
    .WillOnce(::testing::ReturnArg<0>());

  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  // bundle: INSTALLED ---> RESOLVED (Modify)
  //                   ---> STARTING (Modify)
  //                   ---> ACTIVE (Modify)
  EXPECT_CALL(*bundleTracker, AddingBundle).Times(0);
  EXPECT_CALL(*bundleTracker, ModifiedBundle).Times(3);
  EXPECT_CALL(*bundleTracker, RemovedBundle).Times(0);
  bundle.Start();

  // What happens after the bundle is modified is out of scope for this test
  EXPECT_CALL(*bundleTracker, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker->Close();
}

TEST_F(BundleTrackerCustomCallbackTest,
       CallbacksWorkForOverridenTrackerTrackingBundlesAfterBundleRemoved)
{
  auto stateMask = Bundle::State::STATE_INSTALLED;
  auto bundleTracker = std::make_shared<MockBundleTracker>(context, stateMask);
  EXPECT_CALL(*bundleTracker, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";

  EXPECT_CALL(*bundleTracker, AddingBundle).WillOnce(::testing::ReturnArg<0>());
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  // bundle: INSTALLED ---> UNINSTALLED (Remove)
  EXPECT_CALL(*bundleTracker, AddingBundle).Times(0);
  EXPECT_CALL(*bundleTracker, ModifiedBundle).Times(0);
  EXPECT_CALL(*bundleTracker, RemovedBundle).Times(1);
  bundle.Uninstall();

  bundleTracker->Close();
}

//
// Test custom implementation through method override in a BundleTracker subclass tracking custom objects
//

class MockBundleTrackerWithObject : public BundleTracker<int>
{
public:
  MockBundleTrackerWithObject(const BundleContext& context, StateType stateMask)
    : BundleTracker(context, stateMask)
  {}

  MOCK_METHOD(std::optional<int>,
              AddingBundle,
              (const Bundle& bundle, const BundleEvent&),
              (override));
  MOCK_METHOD(void,
              ModifiedBundle,
              (const Bundle&, const BundleEvent&, int),
              (override));
  MOCK_METHOD(void,
              RemovedBundle,
              (const Bundle&, const BundleEvent&, int),
              (override));
};

TEST_F(BundleTrackerCustomCallbackTest,
       ObjectIsTrackedWhenReturnedByAddingBundleFromOverride)
{
  auto bundleTracker = std::make_shared<MockBundleTrackerWithObject>(context, all_states);
  // Make AddingBundle return null for bundles we aren't testing (main, system_bundle)
  EXPECT_CALL(*bundleTracker, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";

  // bundles: ----> INSTALLED (Add)
  EXPECT_CALL(*bundleTracker, AddingBundle)
    .Times(1)
    .WillOnce(::testing::Return(1)); // returns the object 1
  EXPECT_CALL(*bundleTracker, ModifiedBundle).Times(0);
  EXPECT_CALL(*bundleTracker, RemovedBundle).Times(0);
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  EXPECT_EQ(1, bundleTracker->GetObject(bundle))
    << "The object returned by AddingBundle from a BundleTracker subclass should be tracked";

  // Whatever happens after the bundle is added is out of scope for this test
  EXPECT_CALL(*bundleTracker, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker->Close();
}

//
// Test functionality non-specific to customization method
//

TEST_F(BundleTrackerCustomCallbackTest, NoCallbacksWhenClosed)
{
  auto customizer = std::make_shared<MockCustomizer>();
  auto bundleTracker =
    std::make_shared<BundleTracker<>>(context, all_states, customizer);

  // When bundles change states before the BundleTracker is open, there should be no callbacks
  EXPECT_CALL(*customizer, AddingBundle).Times(0);
  EXPECT_CALL(*customizer, ModifiedBundle).Times(0);
  EXPECT_CALL(*customizer, RemovedBundle).Times(0);
  Bundle bundle0 = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleM");
  bundle0.Start();

  // Given a series of callbacks while the BundleTracker is open
  EXPECT_CALL(*customizer, AddingBundle)
    .WillRepeatedly(::testing::ReturnArg<0>());
  EXPECT_CALL(*customizer, ModifiedBundle).Times(::testing::AnyNumber());
  EXPECT_CALL(*customizer, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker->Open();
  Bundle bundle1 = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  bundleTracker->Close();

  // When bundles change states after the BundleTracker is closed, there should be no callbacks
  EXPECT_CALL(*customizer, AddingBundle).Times(0);
  EXPECT_CALL(*customizer, ModifiedBundle).Times(0);
  EXPECT_CALL(*customizer, RemovedBundle).Times(0);
  bundle0.Uninstall();
  bundle1.Start();
  bundle1.Uninstall();
  Bundle bundle2 = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleB");
  bundle2.Uninstall();
}

MATCHER_P(HasName, name, "")
{
  return arg.GetSymbolicName() == name;
}

TEST_F(BundleTrackerCustomCallbackTest,
       CallbacksWorkForBundleFromBeforeTrackerOpened)
{
  // Given a bundle which enters a tracked state before the BundleTracker is opened
  Bundle testBundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  testBundle.Start();
  auto customizer = std::make_shared<MockCustomizer>();
  auto stateMask = Bundle::State::STATE_RESOLVED |
                   Bundle::State::STATE_STARTING | Bundle::State::STATE_ACTIVE;
  auto bundleTracker =
    std::make_shared<BundleTracker<>>(context, stateMask, customizer);
  // and that we ignore callbacks to non-tested bundles
  EXPECT_CALL(*customizer, AddingBundle).Times(::testing::AnyNumber());

  // When the BundleTracker is opened,
  // the only callback for the test bundle should be an AddingBundle
  EXPECT_CALL(*customizer, AddingBundle(HasName("TestBundleA"), ::testing::_))
    .Times(1)
    .WillOnce(::testing::ReturnArg<0>());
  EXPECT_CALL(
    *customizer,
    ModifiedBundle(HasName("TestBundleA"), ::testing::_, ::testing::_))
    .Times(0);
  EXPECT_CALL(*customizer,
              RemovedBundle(HasName("TestBundleA"), ::testing::_, ::testing::_))
    .Times(0);
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";

  // When the test bundle is stopped, we should observe
  //   testBundle : ACTIVE ---> STOPPING (Remove)
  //                       ---> RESOLVED (Add)
  EXPECT_CALL(
    *customizer,
    RemovedBundle(HasName("TestBundleA"), ::testing::_, ::testing::_));
  EXPECT_CALL(*customizer, AddingBundle(HasName("TestBundleA"), ::testing::_))
    .WillOnce(::testing::ReturnArg<0>());
  testBundle.Stop();

  // When the BundleTracker is closed, there should be a RemovedBundle callback
  EXPECT_CALL(*customizer,
              RemovedBundle(HasName("TestBundleA"), ::testing::_, ::testing::_))
    .Times(1);
  bundleTracker->Close();
}