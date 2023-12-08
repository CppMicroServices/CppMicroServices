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
  static constexpr BundleTracker<>::BundleStateMaskType all_states =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE,
                                     Bundle::State::STATE_INSTALLED,
                                     Bundle::State::STATE_RESOLVED,
                                     Bundle::State::STATE_STARTING,
                                     Bundle::State::STATE_STOPPING,
                                     Bundle::State::STATE_UNINSTALLED);

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

MATCHER_P(HasName, name, "")
{
  return arg.GetSymbolicName() == name;
}

//
// Test custom implementation through a
// BundleTrackerCustomizer subclass tracking bundles
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
              (const Bundle&, const BundleEvent&, const Bundle&),
              (override));
  MOCK_METHOD(void,
              RemovedBundle,
              (const Bundle&, const BundleEvent&, const Bundle&),
              (override));
};

TEST_F(BundleTrackerCustomCallbackTest,
       BundleIsTrackedWhenReturnedByAddingBundleFromCustomizer)
{
  // Given an open BundleTracker
  auto stateMask =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE);
  auto customizer = std::make_shared<MockCustomizer>();
  auto bundleTracker = BundleTracker<>(context, stateMask, customizer);
  // where AddingBundle returns null for bundles we aren't testing (main, system_bundle)
  EXPECT_CALL(*customizer, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";

  // When bundleA makes the following state transitions:
  // ---> installed ---> resolved ---> starting ---> ACTIVE (Add)
  // Then there should be 1 AddingBundle callback, and
  // When it returns the bundle
  EXPECT_CALL(*customizer, AddingBundle)
    .Times(1)
    .WillOnce(::testing::ReturnArg<0>()); // returns the bundle
  EXPECT_CALL(*customizer, ModifiedBundle).Times(0);
  EXPECT_CALL(*customizer, RemovedBundle).Times(0);
  Bundle bundleA = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  bundleA.Start();

  // Then the bundle should be the tracked
  EXPECT_EQ(bundleA, bundleTracker.GetObject(bundleA))
    << "The bundle returned by a customizer's AddingBundle should be tracked";

  // At this point, the callbacks after the bundle is added have been verified.
  // We test a different expected behavior below, that RemovedBundle is called on
  // the tracked bundle when the BundleTracker is closed.
  EXPECT_CALL(*customizer, RemovedBundle).Times(1);
  bundleTracker.Close();
}

TEST_F(BundleTrackerCustomCallbackTest,
       BundleUntrackedIfAddingBundleFromCustomizerReturnsNull)
{
  auto customizer = std::make_shared<MockCustomizer>();
  auto bundleTracker = BundleTracker<>(context, all_states, customizer);

  // When AddingBundle callbacks are issued and return null
  EXPECT_CALL(*customizer, AddingBundle)
    .Times(::testing::AtLeast(1))
    .WillRepeatedly(::testing::Return(std::nullopt));
  ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
  Bundle ignoredBundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleM");

  // Then no bundles should be tracked
  EXPECT_EQ(0, bundleTracker.Size())
    << "No bundles should be tracked if AddingBundle always returns null";

  bundleTracker.Close();
}

TEST_F(BundleTrackerCustomCallbackTest,
       CallbacksWorkForCustomizerTrackingBundlesAfterBundleModified)
{
  auto customizer = std::make_shared<MockCustomizer>();
  auto bundleTracker = BundleTracker<>(context, all_states, customizer);
  EXPECT_CALL(*customizer, AddingBundle)
    .WillRepeatedly(::testing::ReturnArg<0>());
  ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  // bundle: INSTALLED ---> RESOLVED (Modify)
  //                   ---> STARTING (Modify)
  //                   ---> ACTIVE (Modify)
  EXPECT_CALL(*customizer, AddingBundle).Times(0);
  EXPECT_CALL(*customizer, ModifiedBundle).Times(3);
  EXPECT_CALL(*customizer, RemovedBundle).Times(0);
  bundle.Start();

  // At this point, the callbacks after the bundle is modified have been verified.
  // The BundleTracker must be closed, after which RemovedBundle is called for
  // all tracked bundles. This behavior is verified in another test, so we do not
  // test it here. Instead, we add an EXPECT_CALL with AnyNumber to retire the
  // previous EXPECT_CALL with Times(0).
  EXPECT_CALL(*customizer, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker.Close();
}

TEST_F(BundleTrackerCustomCallbackTest,
       CallbacksWorkForCustomizerTrackingBundlesAfterBundleRemoved)
{
  auto customizer = std::make_shared<MockCustomizer>();
  auto stateMask =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_INSTALLED);
  auto bundleTracker = BundleTracker<>(context, stateMask, customizer);
  EXPECT_CALL(*customizer, AddingBundle)
    .WillRepeatedly(::testing::ReturnArg<0>());
  ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";

  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  // bundle: INSTALLED ---> uninstalled (Remove)
  EXPECT_CALL(*customizer, AddingBundle).Times(0);
  EXPECT_CALL(*customizer, ModifiedBundle).Times(0);
  EXPECT_CALL(*customizer, RemovedBundle).Times(1);
  bundle.Uninstall();

  // At this point, the callbacks after the bundle is removed have been verified.
  // The BundleTracker must be closed, after which RemovedBundle is called for
  // all tracked bundles. This behavior is verified in another test, so we do not
  // test it here. Instead, we add an EXPECT_CALL with AnyNumber to retire the
  // previous EXPECT_CALL with Times(1).
  EXPECT_CALL(*customizer, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker.Close();
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
              (const Bundle&, const BundleEvent&, const int&),
              (override));
  MOCK_METHOD(void,
              RemovedBundle,
              (const Bundle&, const BundleEvent&, const int&),
              (override));
};

TEST_F(BundleTrackerCustomCallbackTest,
       ObjectIsTrackedWhenReturnedByAddingBundleFromCustomizer)
{
  auto stateMask =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE);
  auto customizer = std::make_shared<MockCustomizerWithObject>();
  auto bundleTracker = BundleTracker<int>(context, stateMask, customizer);

  // Make AddingBundle return null for bundles we aren't testing (main, system_bundle)
  EXPECT_CALL(*customizer, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";

  // When bundle makes the following state transitions:
  // ---> installed ---> resolved ---> starting ---> ACTIVE (Add)
  // Then there should be 1 AddingBundle callback, and
  // When it returns the object 5
  EXPECT_CALL(*customizer, AddingBundle)
    .Times(1)
    .WillOnce(::testing::Return(5)); // returns the object 1
  EXPECT_CALL(*customizer, ModifiedBundle).Times(0);
  EXPECT_CALL(*customizer, RemovedBundle).Times(0);
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  bundle.Start();

  // Then the object 5 should be the tracked
  EXPECT_EQ(5, bundleTracker.GetObject(bundle))
    << "The object returned by a customizer's AddingBundle should be tracked";

  // At this point, the callbacks after the bundle is added have been verified.
  // The BundleTracker must be closed, after which RemovedBundle is called for
  // all tracked bundles. This behavior is verified in another test, so we do not
  // test it here. Instead, we add an EXPECT_CALL with AnyNumber to retire the
  // previous EXPECT_CALL with Times(0).
  EXPECT_CALL(*customizer, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker.Close();
}

//
// Test custom implementation through
// method override in a BundleTracker subclass tracking bundles
//
class MockBundleTracker : public BundleTracker<>
{
public:
  MockBundleTracker(const BundleContext& context, BundleStateMaskType stateMask)
    : BundleTracker(context, stateMask)
  {}

  MOCK_METHOD(std::optional<Bundle>,
              AddingBundle,
              (const Bundle& bundle, const BundleEvent&),
              (override));
  MOCK_METHOD(void,
              ModifiedBundle,
              (const Bundle&, const BundleEvent&, const Bundle&),
              (override));
  MOCK_METHOD(void,
              RemovedBundle,
              (const Bundle&, const BundleEvent&, const Bundle&),
              (override));
};

TEST_F(BundleTrackerCustomCallbackTest,
       BundleIsTrackedWhenReturnedByAddingBundleFromOverride)
{
  // Given an open BundleTracker
  auto stateMask =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE);
  auto bundleTracker = MockBundleTracker(context, stateMask);
  // where AddingBundle returns null for bundles we aren't testing (main, system_bundle)
  EXPECT_CALL(bundleTracker, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";

  // When bundle makes the following state transitions:
  // ---> installed ---> resolved ---> starting ---> ACTIVE (Add)
  // Then there should be 1 AddingBundle callback, and
  // When it returns the bundle
  EXPECT_CALL(bundleTracker, AddingBundle)
    .Times(1)
    .WillOnce(::testing::ReturnArg<0>()); // returns the bundle
  EXPECT_CALL(bundleTracker, ModifiedBundle).Times(0);
  EXPECT_CALL(bundleTracker, RemovedBundle).Times(0);
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  bundle.Start();

  // Then the bundle should be the tracked
  EXPECT_EQ(bundle, bundleTracker.GetObject(bundle))
    << "The bundle returned by AddingBundle from a BundleTracker subclass "
       "should be tracked";

  // At this point, the callbacks after the bundle is added have been verified.
  // The BundleTracker must be closed, after which RemovedBundle is called for
  // all tracked bundles. This behavior is verified in another test, so we do not
  // test it here. Instead, we add an EXPECT_CALL with AnyNumber to retire the
  // previous EXPECT_CALL with Times(0).
  EXPECT_CALL(bundleTracker, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker.Close();
}

TEST_F(BundleTrackerCustomCallbackTest,
       BundleUntrackedIfAddingBundleFromOverrideReturnsNull)
{
  auto bundleTracker = MockBundleTracker(context, all_states);

  // When AddingBundle callbacks are issued and return null
  EXPECT_CALL(bundleTracker, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
  Bundle ignoredBundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleM");

  // Then no bundles should be tracked
  EXPECT_EQ(0, bundleTracker.Size())
    << "No bundles should be tracked if AddingBundle always returns null";

  bundleTracker.Close();
}

TEST_F(BundleTrackerCustomCallbackTest,
       CallbacksWorkForOverridenTrackerTrackingBundlesAfterBundleModified)
{
  auto bundleTracker = MockBundleTracker(context, all_states);
  EXPECT_CALL(bundleTracker, AddingBundle)
    .WillRepeatedly(::testing::ReturnArg<0>());
  ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  // bundle: INSTALLED ---> RESOLVED (Modify)
  //                   ---> STARTING (Modify)
  //                   ---> ACTIVE (Modify)
  EXPECT_CALL(bundleTracker, AddingBundle).Times(0);
  EXPECT_CALL(bundleTracker, ModifiedBundle).Times(3);
  EXPECT_CALL(bundleTracker, RemovedBundle).Times(0);
  bundle.Start();

  // At this point, the callbacks after the bundle is modified have been verified.
  // The BundleTracker must be closed, after which RemovedBundle is called for
  // all tracked bundles. This behavior is verified in another test, so we do not
  // test it here. Instead, we add an EXPECT_CALL with AnyNumber to retire the
  // previous EXPECT_CALL with Times(0).
  EXPECT_CALL(bundleTracker, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker.Close();
}

TEST_F(BundleTrackerCustomCallbackTest,
       CallbacksWorkForOverridenTrackerTrackingBundlesAfterBundleRemoved)
{
  auto stateMask =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_INSTALLED);
  auto bundleTracker = MockBundleTracker(context, stateMask);
  EXPECT_CALL(bundleTracker, AddingBundle)
    .WillRepeatedly(::testing::ReturnArg<0>());
  ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";

  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  // bundle: INSTALLED ---> uninstalled (Remove)
  EXPECT_CALL(bundleTracker, AddingBundle).Times(0);
  EXPECT_CALL(bundleTracker, ModifiedBundle).Times(0);
  EXPECT_CALL(bundleTracker, RemovedBundle).Times(1);
  bundle.Uninstall();

  // At this point, the callbacks after the bundle is removed have been verified.
  // The BundleTracker must be closed, after which RemovedBundle is called for
  // all tracked bundles. This behavior is verified in another test, so we do not
  // test it here. Instead, we add an EXPECT_CALL with AnyNumber to retire the
  // previous EXPECT_CALL with Times(1).
  EXPECT_CALL(bundleTracker, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker.Close();
}

//
// Test custom implementation through
// method override in a BundleTracker subclass tracking custom objects
//
class MockBundleTrackerWithObject : public BundleTracker<int>
{
public:
  MockBundleTrackerWithObject(const BundleContext& context,
                              BundleStateMaskType stateMask)
    : BundleTracker(context, stateMask)
  {}

  MOCK_METHOD(std::optional<int>,
              AddingBundle,
              (const Bundle& bundle, const BundleEvent&),
              (override));
  MOCK_METHOD(void,
              ModifiedBundle,
              (const Bundle&, const BundleEvent&, const int&),
              (override));
  MOCK_METHOD(void,
              RemovedBundle,
              (const Bundle&, const BundleEvent&, const int&),
              (override));
};

TEST_F(BundleTrackerCustomCallbackTest,
       ObjectIsTrackedWhenReturnedByAddingBundleFromOverride)
{
  // Given an open BundleTracker
  auto stateMask =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE);
  auto bundleTracker = MockBundleTrackerWithObject(context, stateMask);
  // where AddingBundle returns null for bundles we aren't testing (main, system_bundle)
  EXPECT_CALL(bundleTracker, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";

  // When bundle makes the following state transitions:
  // ---> installed ---> resolved ---> starting ---> ACTIVE (Add)
  // Then there should be 1 AddingBundle callback, and
  // When it returns the object 5
  EXPECT_CALL(bundleTracker, AddingBundle)
    .Times(1)
    .WillOnce(::testing::Return(5)); // returns the object 5
  EXPECT_CALL(bundleTracker, ModifiedBundle).Times(0);
  EXPECT_CALL(bundleTracker, RemovedBundle).Times(0);
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  bundle.Start();

  // Then the object 5 should be tracked
  EXPECT_EQ(5, bundleTracker.GetObject(bundle))
    << "The object returned by AddingBundle from a BundleTracker subclass "
       "should be tracked";

  // At this point, the callbacks after the bundle is added have been verified.
  // The BundleTracker must be closed, after which RemovedBundle is called for
  // all tracked bundles. This behavior is verified in another test, so we do not
  // test it here. Instead, we add an EXPECT_CALL with AnyNumber to retire the
  // previous EXPECT_CALL with Times(0).
  EXPECT_CALL(bundleTracker, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker.Close();
}

//
// Test functionality non-specific to customization method
//

TEST_F(BundleTrackerCustomCallbackTest, NoCallbacksWhenClosed)
{
  auto customizer = std::make_shared<MockCustomizer>();
  auto bundleTracker = BundleTracker<>(context, all_states, customizer);

  // When bundles change states before the BundleTracker is open, there should be no callbacks
  EXPECT_CALL(*customizer, AddingBundle).Times(0);
  EXPECT_CALL(*customizer, ModifiedBundle).Times(0);
  EXPECT_CALL(*customizer, RemovedBundle).Times(0);
  Bundle testBundle1 = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleM");
  testBundle1.Start();

  // Given a series of callbacks while the BundleTracker is open
  EXPECT_CALL(*customizer, AddingBundle)
    .WillRepeatedly(::testing::ReturnArg<0>());
  EXPECT_CALL(*customizer, ModifiedBundle).Times(::testing::AnyNumber());
  EXPECT_CALL(*customizer, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker.Open();
  Bundle testBundle2 = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  bundleTracker.Close();

  // When bundles change states after the BundleTracker is closed, there should be no callbacks
  EXPECT_CALL(*customizer, AddingBundle).Times(0);
  EXPECT_CALL(*customizer, ModifiedBundle).Times(0);
  EXPECT_CALL(*customizer, RemovedBundle).Times(0);
  testBundle1.Uninstall();
  testBundle2.Start();
  testBundle2.Uninstall();
  cppmicroservices::testing::InstallLib(framework.GetBundleContext(),
                                        "TestBundleB");
}

TEST_F(BundleTrackerCustomCallbackTest,
       CallbacksWorkForBundleFromBeforeTrackerOpened)
{
  // Given a bundle which enters a tracked state before the BundleTracker is opened
  Bundle testBundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  testBundle.Start();
  auto customizer = std::make_shared<MockCustomizer>();
  auto stateMask =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_RESOLVED,
                                     Bundle::State::STATE_STARTING,
                                     Bundle::State::STATE_ACTIVE);
  auto bundleTracker = BundleTracker<>(context, stateMask, customizer);
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
  ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";

  // When the test bundle is stopped, we should observe
  //   testBundle : ACTIVE ---> stopping (Remove) ---> RESOLVED (Add)
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
  bundleTracker.Close();
}

void CreateOpenTracker(Framework framework,
                       BundleContext context,
                       std::shared_ptr<MockCustomizer> customizer)
{
  auto stateMask =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE);
  auto bundleTracker = BundleTracker<>(context, stateMask, customizer);
  EXPECT_CALL(*customizer, AddingBundle)
    .WillRepeatedly(::testing::ReturnArg<0>());
  bundleTracker.Open();
  Bundle testBundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  testBundle.Start();
  // bundleTracker goes out of scope
}

TEST_F(BundleTrackerCustomCallbackTest,
       CloseIsCalledWhenOpenTrackerGoesOutOfScope)
{
  auto customizer = std::make_shared<MockCustomizer>();

  // When an open BundleTracker goes out of scope,
  // Close() should be called, and therefore
  // RemovedBundle should be called on tracked bundles
  EXPECT_CALL(*customizer, RemovedBundle).Times(::testing::AtLeast(1));
  CreateOpenTracker(framework, context, customizer);
}

TEST_F(BundleTrackerCustomCallbackTest, ErrorInAddingBundlePropagates)
{
  auto customizer = std::make_shared<MockCustomizer>();
  auto stateMask =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE);
  auto bundleTracker = BundleTracker<>(context, stateMask, customizer);
  EXPECT_CALL(*customizer, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  bundleTracker.Open();
  bool receivedErrorEvent{ false };
  auto token = framework.GetBundleContext().AddFrameworkListener(
    [&receivedErrorEvent](const FrameworkEvent& evt) {
      if (evt.GetType() == FrameworkEvent::Type::FRAMEWORK_ERROR) {
        receivedErrorEvent = true;
      }
    });

  // When AddingBundle is called and throws an error
  EXPECT_CALL(*customizer, AddingBundle)
    .WillOnce(::testing::Throw(std::runtime_error("foo")));
  cppmicroservices::testing::InstallLib(framework.GetBundleContext(),
                                        "TestBundleA")
    .Start();

  // Then the framework receives it
  EXPECT_TRUE(receivedErrorEvent)
    << "Framework did not receive error after AddingBundle throw";

  EXPECT_CALL(*customizer, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker.Close();
  framework.GetBundleContext().RemoveListener(std::move(token));
}

TEST_F(BundleTrackerCustomCallbackTest, ErrorInModifiedBundlePropagates)
{
  auto customizer = std::make_shared<MockCustomizer>();
  auto stateMask = BundleTracker<>::CreateStateMask(
    Bundle::State::STATE_STARTING, Bundle::State::STATE_ACTIVE);
  auto bundleTracker = BundleTracker<>(context, stateMask, customizer);
  EXPECT_CALL(*customizer, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  bundleTracker.Open();
  bool receivedErrorEvent{ false };
  auto token = framework.GetBundleContext().AddFrameworkListener(
    [&receivedErrorEvent](const FrameworkEvent& evt) {
      if (evt.GetType() == FrameworkEvent::Type::FRAMEWORK_ERROR) {
        receivedErrorEvent = true;
      }
    });

  // When ModifiedBundle is called and throws an error
  EXPECT_CALL(*customizer, AddingBundle).WillOnce(::testing::ReturnArg<0>());
  EXPECT_CALL(*customizer, ModifiedBundle)
    .WillOnce(::testing::Throw(std::runtime_error("foo")));
  cppmicroservices::testing::InstallLib(framework.GetBundleContext(),
                                        "TestBundleA")
    .Start();

  // Then the framework receives it
  EXPECT_TRUE(receivedErrorEvent)
    << "Framework did not receive error after ModifiedBundle throw";

  EXPECT_CALL(*customizer, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker.Close();
  framework.GetBundleContext().RemoveListener(std::move(token));
}

TEST_F(BundleTrackerCustomCallbackTest, ErrorInRemovedBundlePropagates)
{
  auto customizer = std::make_shared<MockCustomizer>();
  auto stateMask =
    BundleTracker<>::CreateStateMask(Bundle::State::STATE_STARTING);
  auto bundleTracker = BundleTracker<>(context, stateMask, customizer);
  EXPECT_CALL(*customizer, AddingBundle)
    .WillRepeatedly(::testing::Return(std::nullopt));
  bundleTracker.Open();
  bool receivedErrorEvent{ false };
  auto token = framework.GetBundleContext().AddFrameworkListener(
    [&receivedErrorEvent](const FrameworkEvent& evt) {
      if (evt.GetType() == FrameworkEvent::Type::FRAMEWORK_ERROR) {
        receivedErrorEvent = true;
      }
    });

  // When RemovedBundle is called and throws an error
  EXPECT_CALL(*customizer, AddingBundle).WillOnce(::testing::ReturnArg<0>());
  EXPECT_CALL(*customizer, RemovedBundle)
    .WillOnce(::testing::Throw(std::runtime_error("foo")));
  cppmicroservices::testing::InstallLib(framework.GetBundleContext(),
                                        "TestBundleA")
    .Start();

  // Then the framework receives it
  EXPECT_TRUE(receivedErrorEvent)
    << "Framework did not receive error after RemovedBundle throw";

  EXPECT_CALL(*customizer, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker.Close();
  framework.GetBundleContext().RemoveListener(std::move(token));
}
