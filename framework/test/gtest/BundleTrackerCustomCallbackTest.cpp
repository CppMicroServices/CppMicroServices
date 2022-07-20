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

TEST_F(BundleTrackerCustomCallbackTest, StartTrackerAfterBundlesOpen)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  ASSERT_NO_THROW(bundleTracker->Open())
    << "BundleTracker failed to start after a bundle was installed";

  bundleTracker->Close();
}

TEST_F(BundleTrackerCustomCallbackTest, NoCallbacksWhenClosed)
{
  //TODO
}

class MockCustomizer : public BundleTrackerCustomizer<>
{
public:
  std::vector<std::string> ignoredBundles = { "main",
                                              "system_bundle",
                                              "TestBundleB" };

  std::optional<Bundle> AddingBundle(const Bundle& bundle,
                                     const BundleEvent&) override
  {
    bool isIgnored =
      std::find(ignoredBundles.begin(),
                ignoredBundles.end(),
                bundle.GetSymbolicName()) != ignoredBundles.end();
    if (isIgnored) {
      return {};
    }
    return bundle;
  }
  //MOCK_METHOD(std::optional<Bundle>,
  //            AddingBundle,
  //            (const Bundle& bundle, const BundleEvent&),
  //            (override));
  MOCK_METHOD(void,
              ModifiedBundle,
              (const Bundle&, const BundleEvent&, Bundle),
              (override));
  MOCK_METHOD(void,
              RemovedBundle,
              (const Bundle&, const BundleEvent&, Bundle),
              (override));
};

class MyCustomizer : public cppmicroservices::BundleTrackerCustomizer<>
{
public:
  int addCount = 0;
  int modifyCount = 0;
  int removeCount = 0;
  std::vector<std::string> ignoredBundles = { "main",
                                              "system_bundle",
                                              "TestBundleB" };

  std::optional<Bundle> AddingBundle(const Bundle& bundle,
                                     const BundleEvent&) override
  {
    bool isIgnored =
      std::find(ignoredBundles.begin(),
                ignoredBundles.end(),
                bundle.GetSymbolicName()) != ignoredBundles.end();
    if (isIgnored) {
      return {};
    }
    addCount += 1;
    return bundle;
  }

  void ModifiedBundle(const Bundle&, const BundleEvent&, Bundle) override
  {
    modifyCount += 1;
  }

  void RemovedBundle(const Bundle&, const BundleEvent&, Bundle) override
  {
    removeCount += 1;
  }
};

TEST_F(BundleTrackerCustomCallbackTest,
       BundleIsTrackedWhenReturnedByAddingBundleFromCustomizer)
{
  std::shared_ptr<MyCustomizer> custom = std::make_shared<MyCustomizer>();
  auto bundleTracker =
    std::make_shared<BundleTracker<>>(context, all_states, custom);
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";

  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  bundle.Start();
  bundle.Uninstall();

  EXPECT_EQ(bundle, bundleTracker->GetObject(bundle))
    << "The object returned by custom AddingBundle from "
       "BundleTrackerCustomizer was not tracked";

  bundleTracker->Close();
}

TEST_F(BundleTrackerCustomCallbackTest,
       BundleUntrackedIfAddingBundleFromCustomizerReturnsNull)
{
  auto customizer = std::make_shared<MockCustomizer>();
  auto bundleTracker =
    std::make_shared<BundleTracker<>>(context, all_states, customizer);
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";

  Bundle ignoredBundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleB");

  EXPECT_EQ(0, bundleTracker->Size());

  bundleTracker->Close();
}

TEST_F(BundleTrackerCustomCallbackTest,
       CallbacksWorkForCustomizerTrackingBundlesAfterBundleModified)
{
  auto customizer = std::make_shared<MockCustomizer>();
  auto bundleTracker =
    std::make_shared<BundleTracker<>>(context, all_states, customizer);
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  EXPECT_CALL(*customizer, ModifiedBundle).Times(3);
  EXPECT_CALL(*customizer, RemovedBundle).Times(0);
  bundle.Start();

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
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  // bundle: Installed --(Remove)--> Uninstalled
  EXPECT_CALL(*customizer, ModifiedBundle).Times(0);
  EXPECT_CALL(*customizer, RemovedBundle).Times(1);
  bundle.Uninstall();

  bundleTracker->Close();
}

//
// Test custom implementation through a BundleTrackerCustomizer subclass tracking custom objects
//

TEST_F(BundleTrackerCustomCallbackTest,
       ObjectIsTrackedWhenReturnedByAddingBundleFromCustomizer)
{
  //TODO
}

TEST_F(BundleTrackerCustomCallbackTest,
       ObjectUntrackedIfAddingBundleFromCustomizerReturnsNull)
{
  //TODO
}

TEST_F(BundleTrackerCustomCallbackTest,
       ModifiedBundleFromCustomizerTrackingObjectsCallbackWorks)
{
  //TODO
}

TEST_F(BundleTrackerCustomCallbackTest,
       RemovedBundleFromCustomizerTrackingObjectsCallbackWorks)
{
  //TODO
}

class MyCustomizer2
  : public cppmicroservices::BundleTrackerCustomizer<std::string>
{
public:
  int addCount = 0;
  int modifyCount = 0;
  int removeCount = 0;
  std::vector<std::string> ignoredBundles = { "main",
                                              "system_bundle",
                                              "TestBundleB" };

  std::optional<std::string> AddingBundle(const Bundle& bundle,
                                          const BundleEvent&) override
  {
    bool isIgnored =
      std::find(ignoredBundles.begin(),
                ignoredBundles.end(),
                bundle.GetSymbolicName()) != ignoredBundles.end();
    if (isIgnored) {
      return {};
    }
    addCount += 1;
    return bundle.GetSymbolicName();
  }

  void ModifiedBundle(const Bundle&,
                      const BundleEvent& event,
                      std::string str) override
  {
    switch (event.GetType()) {
      case BundleEvent::BUNDLE_RESOLVED:
        str.append("-resolved");
        break;

      case BundleEvent::BUNDLE_STARTING:
        str.append("-starting");
        break;

      case BundleEvent::BUNDLE_STARTED:
        str.append("-started");
        break;

      default:
        str.append("-other");
    }
    modifyCount += 1;
  }

  void RemovedBundle(const Bundle&, const BundleEvent&, std::string) override
  {
    removeCount += 1;
  }
};

//TEST_F(BundleTrackerCustomCallbackTest,
//       ObjectIsTrackedWhenReturnedByCustomAddingBundleFromCustomizer)
//{
//  std::shared_ptr<MyCustomizer2> custom = std::make_shared<MyCustomizer2>();
//  auto bundleTracker =
//    std::make_shared<BundleTracker<std::string>>(context, all_states, custom);
//  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
//
//  Bundle bundle = cppmicroservices::testing::InstallLib(
//    framework.GetBundleContext(), "TestBundleA");
//  bundle.Start();
//  bundle.Uninstall();
//  EXPECT_EQ("TestBundleA", bundleTracker->GetObject(bundle))
//    << "The custom object returned by custom AddingBundle from "
//       "BundleTrackerCustomizer was not tracked";
//
//  bundleTracker->Close();
//}

//TEST_F(BundleTrackerCustomCallbackTest,
//       BundleUntrackedIfCustomAddingBundleFromCustomizerReturnsNull)
//{
//  std::shared_ptr<MyCustomizer2> custom = std::make_shared<MyCustomizer2>();
//  auto bundleTracker =
//    std::make_shared<BundleTracker<std::string>>(context, all_states, custom);
//  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
//
//  Bundle ignoredBundle = cppmicroservices::testing::InstallLib(
//    framework.GetBundleContext(), "TestBundleB");
//  ignoredBundle.Start();
//  ignoredBundle.Uninstall();
//
//  /*EXPECT_EQ(nullptr, bundleTracker->GetObject(ignoredBundle))
//    << "Custom AddingBundle from BundleTrackerCustomizer returned null but the "
//       "bundle was still tracked";*/
//
//  bundleTracker->Close();
//}

//TEST_F(BundleTrackerCustomCallbackTest,
//       CustomModifiedBundleFromCustomizerCallbackWorks)
//{
//  std::shared_ptr<MyCustomizer2> custom = std::make_shared<MyCustomizer2>();
//  auto bundleTracker =
//    std::make_shared<BundleTracker<std::string>>(context, all_states, custom);
//  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
//
//  Bundle bundle = cppmicroservices::testing::InstallLib(
//    framework.GetBundleContext(), "TestBundleA");
//  bundle.Start();
//
//  EXPECT_EQ("TestBundleA-resolved-starting-started",
//            bundleTracker->GetObject(bundle))
//    << "Callbacks to custom ModifiedBundle from BundleTrackerCustomizer were "
//       "not issued correctly";
//
//  bundleTracker->Close();
//}

//TEST_F(BundleTrackerCustomCallbackTest,
//       CustomRemovedBundleFromCustomizerCallbackWorks)
//{
//  std::shared_ptr<MyCustomizer2> custom = std::make_shared<MyCustomizer2>();
//  auto stateMask = all_states - Bundle::State::STATE_UNINSTALLED;
//  auto bundleTracker =
//    std::make_shared<BundleTracker<std::string>>(context, stateMask, custom);
//  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
//
//  Bundle bundle = cppmicroservices::testing::InstallLib(
//    framework.GetBundleContext(), "TestBundleA");
//  bundle.Start();
//  bundle.Uninstall();
//
//  EXPECT_LE(1, custom->removeCount)
//    << "More than 1 callback was issued to custom RemovedBundle from "
//       "BundleTrackerCustomizer for a single removal";
//  EXPECT_GE(1, custom->removeCount)
//    << "No callbacks were issued to custom RemovedBundle from "
//       "BundleTrackerCustomizer after a removal";
//
//  bundleTracker->Close();
//}

class MockBundleTracker : public BundleTracker<>
{
public:
  std::vector<std::string> ignoredBundles = { "main",
                                              "system_bundle",
                                              "TestBundleB" };

  MockBundleTracker(const BundleContext& context, StateType stateMask)
    : BundleTracker(context, stateMask)
  {}

  std::optional<Bundle> AddingBundle(const Bundle& bundle,
                                     const BundleEvent&) override
  {
    bool isIgnored =
      std::find(ignoredBundles.begin(),
                ignoredBundles.end(),
                bundle.GetSymbolicName()) != ignoredBundles.end();
    if (isIgnored) {
      return {};
    }
    return bundle;
  }
  //MOCK_METHOD(std::optional<Bundle>,
  //            AddingBundle,
  //            (const Bundle& bundle, const BundleEvent&),
  //            (override));
  MOCK_METHOD(void,
              ModifiedBundle,
              (const Bundle&, const BundleEvent&, Bundle),
              (override));
  MOCK_METHOD(void,
              RemovedBundle,
              (const Bundle&, const BundleEvent&, Bundle),
              (override));
};

//class MyBundleTracker : public BundleTracker<std::string>
//{
//public:
//  int addCount = 0;
//  int modifyCount = 0;
//  int removeCount = 0;
//  std::vector<std::string> ignoredBundles = { "main",
//                                              "system_bundle",
//                                              "TestBundleB" };
//
//  std::optional<std::string> AddingBundle(const Bundle& bundle,
//                                          const BundleEvent&) override
//  {
//    bool isIgnored =
//      std::find(ignoredBundles.begin(),
//                ignoredBundles.end(),
//                bundle.GetSymbolicName()) != ignoredBundles.end();
//    if (isIgnored) {
//      return {};
//    }
//    addCount += 1;
//    return bundle.GetSymbolicName();
//  }
//
//  void ModifiedBundle(const Bundle&,
//                      const BundleEvent& event,
//                      std::string str) override
//  {
//    switch (event.GetType()) {
//      case BundleEvent::BUNDLE_RESOLVED:
//        str.append("-resolved");
//        break;
//
//      case BundleEvent::BUNDLE_STARTING:
//        str.append("-starting");
//        break;
//
//      case BundleEvent::BUNDLE_STARTED:
//        str.append("-started");
//        break;
//
//      default:
//        str.append("-other");
//    }
//    modifyCount += 1;
//  }
//
//  void RemovedBundle(const Bundle&, const BundleEvent&, std::string) override
//  {
//    removeCount += 1;
//  }
//};

TEST_F(BundleTrackerCustomCallbackTest,
       BundleIsTrackedWhenReturnedByAddingBundleFromOverride)
{
  //TODO
}

TEST_F(BundleTrackerCustomCallbackTest,
       BundleUntrackedIfAddingBundleFromOverrideReturnsNull)
{
  //TODO
}

TEST_F(BundleTrackerCustomCallbackTest,
       ModifiedBundleFromOverrideTrackingBundlesCallbackWorks)
{
  auto bundleTracker = std::make_shared<MockBundleTracker>(context, all_states);
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";

  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  EXPECT_CALL(*bundleTracker, ModifiedBundle).Times(3);
  EXPECT_CALL(*bundleTracker, RemovedBundle).Times(0);
  bundle.Start();

  EXPECT_CALL(*bundleTracker, RemovedBundle).Times(::testing::AnyNumber());
  bundleTracker->Close();
}

TEST_F(BundleTrackerCustomCallbackTest,
       RemovedBundleFromOverrideTrackingBundlesCallbackWorks)
{
  //TODO
}

//
//
//

TEST_F(BundleTrackerCustomCallbackTest,
       ObjectIsTrackedWhenReturnedByAddingBundleFromOverride)
{
  //TODO
}

TEST_F(BundleTrackerCustomCallbackTest,
       ObjectUntrackedIfAddingBundleFromOverrideReturnsNull)
{
  //TODO
}

TEST_F(BundleTrackerCustomCallbackTest,
       ModifiedBundleFromOverrideTrackingObjectsCallbackWorks)
{
  //TODO
}

TEST_F(BundleTrackerCustomCallbackTest,
       RemovedBundleFromOverrideTrackingObjectsCallbackWorks)
{
  //TODO
}
