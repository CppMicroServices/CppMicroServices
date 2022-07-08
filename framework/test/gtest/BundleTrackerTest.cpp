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
#include "cppmicroservices/BundleTracker.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "TestUtils.h"
#include "gtest/gtest.h"

using namespace cppmicroservices;

class BundleTrackerTest : public ::testing::Test
{
protected:
  Framework framework;
  BundleContext context;

public:
  BundleTrackerTest()
    : framework(FrameworkFactory().NewFramework()){};

  ~BundleTrackerTest() override = default;

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

auto all_states =
  Bundle::State::STATE_ACTIVE | Bundle::State::STATE_INSTALLED |
  Bundle::State::STATE_RESOLVED | Bundle::State::STATE_STARTING |
  Bundle::State::STATE_STOPPING | Bundle::State::STATE_UNINSTALLED;

TEST_F(BundleTrackerTest, CreateTracker)
{
  BundleTracker bundleTracker(context, all_states);
  EXPECT_TRUE(bundleTracker.IsEmpty());
  EXPECT_EQ(0, bundleTracker.Size());
}

TEST_F(BundleTrackerTest, TestGetBundles)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  std::vector<Bundle> bundles = bundleTracker->GetBundles();
  EXPECT_EQ(0, bundles.size()) << "No bundles should be tracked before Open()";
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

  EXPECT_TRUE(mainTracked);
  EXPECT_TRUE(sysTracked);
  EXPECT_TRUE(bundleATracked);
  EXPECT_EQ(3, bundles.size());

  bundleTracker->Close();
  bundles = bundleTracker->GetBundles();
  EXPECT_EQ(0, bundles.size()) << "No bundles should be tracked after Close()";
}

TEST_F(BundleTrackerTest, TestGetTracked)
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
  EXPECT_EQ(3, tracked.size());

  bundleTracker->Close();
  EXPECT_EQ(0, tracked.size()) << "No objects should be tracked after Close()";
}

TEST_F(BundleTrackerTest, TestIsEmpty)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  EXPECT_TRUE(bundleTracker->IsEmpty()) << "Unopened BundleTracker should be empty";

  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  EXPECT_FALSE(bundleTracker->IsEmpty());

  bundleTracker->Close();
  EXPECT_TRUE(bundleTracker->IsEmpty()) << "Closed BundleTracker should be empty";
}

TEST_F(BundleTrackerTest, TestSize)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  EXPECT_EQ(0, bundleTracker->Size()) << "Size of unopened BundleTracker was not 0";

  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  EXPECT_EQ(2, bundleTracker->Size());

  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  EXPECT_EQ(3, bundleTracker->Size());

  bundleTracker->Close();
  EXPECT_EQ(0, bundleTracker->Size()) << "Size of closed BundleTracker was not 0";
}

TEST_F(BundleTrackerTest, TestGetTrackingCountClosed)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  EXPECT_EQ(-1, bundleTracker->GetTrackingCount()) << "Tracking count of unopened BundleTracker was not -1";
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  bundleTracker->Close();
  //EXPECT_EQ(-1, bundleTracker->GetTrackingCount()); Expected behavior after Close() to be clarified
}

TEST_F(BundleTrackerTest, TestGetTrackingCountAdd)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  int trackingCount0 = bundleTracker->GetTrackingCount();
  cppmicroservices::testing::InstallLib(framework.GetBundleContext(),
                                        "TestBundleA");
  int trackingCount1 = bundleTracker->GetTrackingCount();
  EXPECT_EQ(1, trackingCount1 - trackingCount0)
    << "Tracking count didn't increment by 1 after a bundle was added";
  bundleTracker->Close();
}

TEST_F(BundleTrackerTest, TestGetTrackingCountModify)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
  ASSERT_NO_THROW(bundleTracker->Open()) << "BundleTracker failed to start";
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  ASSERT_EQ(Bundle::State::STATE_INSTALLED, bundle.GetState()) << "Test bundle failed to install";
  int trackingCount0 = bundleTracker->GetTrackingCount();

  bundle.Start(); // bundle: INSTALLED(T)->RESOLVED(T)->STARTING(T)->ACTIVE(T)
  ASSERT_EQ(Bundle::State::STATE_ACTIVE, bundle.GetState()) << "Test bundle failed to start";
  int trackingCount1 = bundleTracker->GetTrackingCount();

  EXPECT_EQ(3, trackingCount1 - trackingCount0)
    << "Tracking count didn't increment by 1 after a bundle was modified";
  bundleTracker->Close();
}

TEST_F(BundleTrackerTest, TestGetTrackingCountRemove)
{
  auto stateMask = Bundle::State::STATE_INSTALLED;
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

TEST_F(BundleTrackerTest, TestGetTrackingCountRemoveMethod)
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

TEST_F(BundleTrackerTest, TestRemove)
{
  auto bundleTracker = std::make_shared<BundleTracker<>>(context, all_states);
}

TEST_F(BundleTrackerTest, TestOpenOpened)
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

TEST_F(BundleTrackerTest, DemoTest)
{
  Bundle bundle;
  ASSERT_FALSE(bundle);

  bundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(),
                                                 "TestBundleA");
  ASSERT_NO_THROW(
    BundleEvent(BundleEvent::Type::BUNDLE_INSTALLED, bundle, bundle));
  ASSERT_TRUE(bundle);
  ASSERT_EQ(bundle.GetSymbolicName(), "TestBundleA");
  ASSERT_EQ(cppmicroservices::Bundle::State::STATE_INSTALLED,
            bundle.GetState());

  bundle.Start();
  ASSERT_EQ(cppmicroservices::Bundle::State::STATE_ACTIVE, bundle.GetState());

  bundle.Stop();
  ASSERT_EQ(cppmicroservices::Bundle::State::STATE_RESOLVED, bundle.GetState());

  bundle.Uninstall();
  ASSERT_EQ(cppmicroservices::Bundle::State::STATE_UNINSTALLED,
            bundle.GetState());
  ASSERT_THROW(bundle.Start(), std::logic_error);
}
