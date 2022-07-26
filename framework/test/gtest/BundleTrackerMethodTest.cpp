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
#include "gtest/gtest.h"

#ifdef GetObject
#  undef GetObject
#endif

using namespace cppmicroservices;

class BundleTrackerMethodTest : public ::testing::Test
{
protected:
  Framework framework;
  BundleContext context;
  BundleTracker<>::BundleState all_states =
    _CreateStateMask(Bundle::State::STATE_ACTIVE,
                     Bundle::State::STATE_INSTALLED,
                     Bundle::State::STATE_RESOLVED,
                     Bundle::State::STATE_STARTING,
                     Bundle::State::STATE_STOPPING,
                     Bundle::State::STATE_UNINSTALLED);

public:
  BundleTrackerMethodTest()
    : framework(FrameworkFactory().NewFramework()){};

  ~BundleTrackerMethodTest() override = default;

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

TEST_F(BundleTrackerMethodTest, CanCreateTracker)
{
  ASSERT_NO_THROW(BundleTracker bundleTracker(context, all_states))
    << "Creation of BundleTracker failed";
}

TEST_F(BundleTrackerMethodTest, GetBundlesWorks)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  std::vector<Bundle> bundles = bundleTracker->GetBundles();
  EXPECT_EQ(0, bundles.size())
    << "GetBundles() should return an empty vector before Open()";

  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  Bundle bundleMain = cppmicroservices::testing::GetBundle("main", context);
  Bundle bundleSys =
    cppmicroservices::testing::GetBundle("system_bundle", context);
  Bundle bundleA = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  bundles = bundleTracker->GetBundles();

  bool mainTracked =
    std::find(bundles.begin(), bundles.end(), bundleMain) != bundles.end();
  bool sysTracked =
    std::find(bundles.begin(), bundles.end(), bundleSys) != bundles.end();
  bool bundleATracked =
    std::find(bundles.begin(), bundles.end(), bundleA) != bundles.end();

  EXPECT_TRUE(mainTracked) << "GetBundles() should include the main bundle";
  EXPECT_TRUE(sysTracked) << "GetBundles() should include the system bundle";
  EXPECT_TRUE(bundleATracked) << "GetBundles() should include the test bundle";

  bundleTracker->Close();
  bundles = bundleTracker->GetBundles();
  EXPECT_EQ(0, bundles.size())
    << "GetBundles() should return an empty vector after Close()";
}

TEST_F(BundleTrackerMethodTest, GetObjectWorks)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  Bundle bundleA = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  EXPECT_EQ(bundleA, bundleTracker->GetObject(bundleA));
  // TODO: test a case where GetObject returns null
}

TEST_F(BundleTrackerMethodTest, GetTrackedWorks)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  auto tracked = bundleTracker->GetTracked();
  EXPECT_EQ(0, tracked.size()) << "No objects should be tracked before Open()";

  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  Bundle bundleMain = cppmicroservices::testing::GetBundle("main", context);
  Bundle bundleSys =
    cppmicroservices::testing::GetBundle("system_bundle", context);
  Bundle bundleA = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  tracked = bundleTracker->GetTracked();

  bool mainTracked = tracked.find(bundleMain) != tracked.end();
  bool sysTracked = tracked.find(bundleSys) != tracked.end();
  bool bundleATracked = tracked.find(bundleA) != tracked.end();

  EXPECT_TRUE(mainTracked);
  EXPECT_TRUE(sysTracked);
  EXPECT_TRUE(bundleATracked);

  bundleTracker->Close();
  tracked = bundleTracker->GetTracked();
  EXPECT_EQ(0, tracked.size()) << "No objects should be tracked after Close()";
}

TEST_F(BundleTrackerMethodTest, IsEmptyWorks)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  EXPECT_TRUE(bundleTracker->IsEmpty())
    << "Unopened BundleTracker should be empty";

  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  EXPECT_FALSE(bundleTracker->IsEmpty());

  bundleTracker->Close();
  EXPECT_TRUE(bundleTracker->IsEmpty())
    << "Closed BundleTracker should be empty";
}

TEST_F(BundleTrackerMethodTest, SizeWorks)
{
  auto stateMask = _CreateStateMask(Bundle::State::STATE_UNINSTALLED);
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, stateMask);
  EXPECT_EQ(0, bundleTracker->Size())
    << "Size of unopened BundleTracker was not 0";

  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  EXPECT_EQ(0, bundleTracker->Size())
    << "Size should be 0 when no bundles has entered a tracked state";

  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  bundle.Uninstall();
  EXPECT_EQ(1, bundleTracker->Size())
    << "Size should be 1 when 1 bundle entered a tracked state";

  bundleTracker->Close();
  EXPECT_EQ(0, bundleTracker->Size())
    << "Size of closed BundleTracker was not 0";
}

TEST_F(BundleTrackerMethodTest, GetTrackingCountWorksAfterBundleAdded)
{
  auto stateMask = _CreateStateMask(Bundle::State::STATE_ACTIVE);
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, stateMask);
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  int trackingCount0 = bundleTracker->GetTrackingCount();

  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  bundle.Start();
  int trackingCount1 = bundleTracker->GetTrackingCount();

  EXPECT_EQ(1, trackingCount1 - trackingCount0)
    << "Tracking count didn't increment by 1 after a bundle was added";
  bundleTracker->Close();
}

TEST_F(BundleTrackerMethodTest, GetTrackingCountWorksAfterBundleModified)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  ASSERT_EQ(Bundle::State::STATE_INSTALLED, bundle.GetState())
    << "Test bundle failed to install";
  int trackingCount0 = bundleTracker->GetTrackingCount();

  bundle.Start(); // bundle: INSTALLED(T)->RESOLVED(T)->STARTING(T)->ACTIVE(T)
  ASSERT_EQ(Bundle::State::STATE_ACTIVE, bundle.GetState())
    << "Test bundle failed to start";
  int trackingCount1 = bundleTracker->GetTrackingCount();

  EXPECT_EQ(3, trackingCount1 - trackingCount0)
    << "Tracking count didn't increment by 3 after a bundle was modified 3 "
       "times";
  bundleTracker->Close();
}

TEST_F(BundleTrackerMethodTest,
       GetTrackingCountWorksAfterBundleRemovedByStateChange)
{
  auto stateMask = _CreateStateMask(Bundle::State::STATE_INSTALLED);
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, stateMask);
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  ASSERT_EQ(Bundle::State::STATE_INSTALLED, bundle.GetState())
    << "Test bundle failed to install";
  int trackingCount0 = bundleTracker->GetTrackingCount();

  bundle.Start(); // bundle: INSTALLED(T)->RESOLVED(U)->STARTING(U)->ACTIVE(U)
  ASSERT_EQ(Bundle::State::STATE_ACTIVE, bundle.GetState())
    << "Test bundle failed to start";
  int trackingCount1 = bundleTracker->GetTrackingCount();

  EXPECT_EQ(1, trackingCount1 - trackingCount0)
    << "Tracking count didn't increment by 1 after a bundle was removed";
  bundleTracker->Close();
}

TEST_F(BundleTrackerMethodTest, GetTrackingCountWorksAfterRemoveMethod)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  ASSERT_EQ(Bundle::State::STATE_INSTALLED, bundle.GetState())
    << "Test bundle failed to install";
  int trackingCount0 = bundleTracker->GetTrackingCount();

  // If Remove removes a bundle from being tracked, tracking count increments by 1
  bundleTracker->Remove(bundle);
  ASSERT_EQ(Bundle::State::STATE_INSTALLED, bundle.GetState())
    << "Remove() altered the bundle being removed";
  int trackingCount1 = bundleTracker->GetTrackingCount();

  EXPECT_EQ(1, trackingCount1 - trackingCount0)
    << "Tracking count didn't increment by 1 after Remove() removed a bundle";

  // If Remove doesn't remove a bundle from being tracked, tracking count stays the same
  bundleTracker->Remove(bundle);
  int trackingCount2 = bundleTracker->GetTrackingCount();

  EXPECT_EQ(0, trackingCount2 - trackingCount1)
    << "Tracking count changed after Remove() didn't remove a bundle";
  bundleTracker->Close();
}

TEST_F(BundleTrackerMethodTest, GetTrackingCountWorksWhenClosed)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  EXPECT_EQ(-1, bundleTracker->GetTrackingCount())
    << "Tracking count of unopened BundleTracker was not -1";
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  bundleTracker->Close();
  //EXPECT_EQ(-1, bundleTracker->GetTrackingCount()); Expected behavior after Close() to be clarified
}

TEST_F(BundleTrackerMethodTest, RemoveWorks)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  auto bundlesTracked = bundleTracker->GetBundles();
  bool mainTracked =
    std::find(bundlesTracked.begin(), bundlesTracked.end(), bundle) !=
    bundlesTracked.end();
  ASSERT_TRUE(mainTracked) << "BundleTracker failed to track a bundle";

  bundleTracker->Remove(bundle);
  bundlesTracked = bundleTracker->GetBundles();
  bool mainStillTracked =
    std::find(bundlesTracked.begin(), bundlesTracked.end(), bundle) !=
    bundlesTracked.end();

  EXPECT_FALSE(mainStillTracked)
    << "Calling Remove() didn't removed tracked bundle from being tracked";
}

TEST_F(BundleTrackerMethodTest, BundleRemovedByRemoveTrackable)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  Bundle testBundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");

  bundleTracker->Remove(testBundle);
  testBundle.Start();

  bool testBundleTracked = bundleTracker->GetObject(testBundle) == testBundle;
  EXPECT_TRUE(testBundleTracked)
    << "Remove() was called on a tracked bundle, then later entered a tracked "
       "state. The bundle should be tracked but wasn't ";
}

TEST_F(BundleTrackerMethodTest, DefaultAddingBundleReturnsCorrectValue)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  Bundle bundle = Bundle();
  EXPECT_EQ(bundle, bundleTracker->AddingBundle(bundle, BundleEvent()));
}

TEST_F(BundleTrackerMethodTest, RemoveUntrackedBundleDoesNothing)
{
  BundleTracker<> bundleTracker =
    BundleTracker<>(context, Bundle::State::STATE_ACTIVE);
  ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  int preCount = bundleTracker.GetTrackingCount();

  ASSERT_NO_THROW(bundleTracker.Remove(bundle))
    << "Manually removing untracked Bundle should not throw";
  int postCount = bundleTracker.GetTrackingCount();
  EXPECT_EQ(preCount, postCount)
    << "BundleTracker should no-op when untracked Bundle is manually removed";

  bundleTracker.Close();
}

TEST_F(BundleTrackerMethodTest, ReopeningTrackerWorks)
{
  auto stateMask = _CreateStateMask(Bundle::State::STATE_ACTIVE);
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, stateMask);
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  bundle.Start();
  ASSERT_EQ(2, bundleTracker->Size());
  bundleTracker->Close();

  ASSERT_NO_THROW(bundleTracker->Open())
    << "BundleTracker failed to start after it was previously opened and "
       "closed";
  EXPECT_EQ(2, bundleTracker->Size())
    << "2 bundles should be tracked after tracker is reopened";

  bundleTracker->Close();
}

TEST_F(BundleTrackerMethodTest, BundlesInUntrackedStatesUntracked)
{
  auto stateMask = _CreateStateMask(Bundle::State::STATE_STARTING,
                                    Bundle::State::STATE_STOPPING);
  BundleTracker<> bundleTracker = BundleTracker<>(context, stateMask);
  ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";

  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  bundle.Start();

  EXPECT_EQ(0, bundleTracker.Size())
    << "No bundles should be tracked when none are in a state covered by the "
       "state mask";
  bundleTracker.Close();
}

TEST_F(BundleTrackerMethodTest, OpenWithInvalidContextThrowsError)
{
  auto invalidContext = BundleContext();
  BundleTracker<> bundleTracker = BundleTracker<>(invalidContext, all_states);

  EXPECT_THROW(bundleTracker.Open(), std::runtime_error)
    << "Opening BundleTracker with invalid context should throw";
}

TEST_F(BundleTrackerMethodTest, OpeningOpenTrackerDoesNothing)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  bundle.Start();
  int trackingCount0 = bundleTracker->GetTrackingCount();
  int s0 = bundleTracker->Size();

  ASSERT_NO_THROW(bundleTracker->Open())
    << "Open() on opened BundleTracker threw an error";

  int trackingCount1 = bundleTracker->GetTrackingCount();
  int s1 = bundleTracker->Size();
  EXPECT_EQ(0, trackingCount1 - trackingCount0)
    << "Open() on opened BundleTracker increased the tracking count";
  EXPECT_EQ(0, s1 - s0) << "Open() on opened BundleTracker changed the size";
  bundleTracker->Close();
}

TEST_F(BundleTrackerMethodTest, ClosingClosedTrackerDoesNothing)
{
  BundleTracker<> bundleTracker = BundleTracker<>(context, all_states);
  ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
  ASSERT_NO_THROW(bundleTracker.Close()) << "BundleTracker failed to close";
  int preCount = bundleTracker.GetTrackingCount();

  ASSERT_NO_THROW(bundleTracker.Close())
    << "Closing closed BundleTracker should no-op";
  int postCount = bundleTracker.GetTrackingCount();
  EXPECT_EQ(preCount, postCount) << "Closing closed BundleTracker should no-op";
}

TEST_F(BundleTrackerMethodTest, ClosingUnopenTrackerDoesNothing)
{
  BundleTracker<> bundleTracker = BundleTracker<>(context, all_states);
  int preCount = bundleTracker.GetTrackingCount();

  ASSERT_NO_THROW(bundleTracker.Close())
    << "Closing unopened BundleTracker should no-op";
  int postCount = bundleTracker.GetTrackingCount();
  EXPECT_EQ(preCount, postCount)
    << "Closing unopened BundleTracker should no-op";
}
