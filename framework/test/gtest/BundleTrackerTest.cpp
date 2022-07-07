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
  BundleTracker bt(context, all_states);
  EXPECT_TRUE(bt.IsEmpty());
  EXPECT_EQ(0, bt.Size());
}

TEST_F(BundleTrackerTest, TestIsEmpty)
{
  auto bt = std::make_shared<BundleTracker<>>(context, all_states);
  EXPECT_TRUE(bt->IsEmpty());

  bt->Open();
  EXPECT_FALSE(bt->IsEmpty());

  bt->Close();
  EXPECT_TRUE(bt->IsEmpty());
}

TEST_F(BundleTrackerTest, TestGetTrackingCountOpened)
{
  auto bt = std::make_shared<BundleTracker<>>(context,
                                              Bundle::State::STATE_UNINSTALLED);
  bt->Open();
  //EXPECT_EQ(0, bt->GetTrackingCount()); TODO clarify expected value
  bt->Close();
}

TEST_F(BundleTrackerTest, TestGetTrackingCountClosed)
{
  auto bt = std::make_shared<BundleTracker<>>(context, all_states);
  EXPECT_EQ(-1, bt->GetTrackingCount());
  bt->Open();
  bt->Close();
  EXPECT_EQ(-1, bt->GetTrackingCount());
}

TEST_F(BundleTrackerTest, TestGetTrackingCountAdd)
{
  auto bt = std::make_shared<BundleTracker<>>(context, all_states);
  bt->Open();
  int tc0 = bt->GetTrackingCount();
  cppmicroservices::testing::InstallLib(framework.GetBundleContext(),
                                        "TestBundleA");
  int tc1 = bt->GetTrackingCount();
  EXPECT_EQ(1, tc1 - tc0);
  bt->Close();
}

TEST_F(BundleTrackerTest, TestGetTrackingCountModify)
{
  auto bt = std::make_shared<BundleTracker<>>(context, all_states);
  bt->Open();
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  ASSERT_EQ(Bundle::State::STATE_INSTALLED, bundle.GetState());
  int tc0 = bt->GetTrackingCount();

  bundle.Start(); // bundle: INSTALLED(T)->RESOLVED(T)->STARTING(T)->ACTIVE(T)
  ASSERT_EQ(Bundle::State::STATE_ACTIVE, bundle.GetState());
  int tc1 = bt->GetTrackingCount();

  EXPECT_EQ(3, tc1 - tc0);
  bt->Close();
}

TEST_F(BundleTrackerTest, TestGetTrackingCountRemove)
{
  auto stateMask = Bundle::State::STATE_INSTALLED;
  auto bt = std::make_shared<BundleTracker<>>(context, stateMask);
  bt->Open();
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  ASSERT_EQ(Bundle::State::STATE_INSTALLED, bundle.GetState());
  int tc0 = bt->GetTrackingCount();

  bundle.Start(); // bundle: INSTALLED(T)->RESOLVED(U)->STARTING(U)->ACTIVE(U)
  ASSERT_EQ(Bundle::State::STATE_ACTIVE, bundle.GetState());
  int tc1 = bt->GetTrackingCount();

  EXPECT_EQ(1, tc1 - tc0);
  bt->Close();
}

TEST_F(BundleTrackerTest, TestGetTrackingCountRemoveMethod)
{
  auto bt = std::make_shared<BundleTracker<>>(context, all_states);
  bt->Open();
  Bundle bundle = cppmicroservices::testing::InstallLib(
    framework.GetBundleContext(), "TestBundleA");
  ASSERT_EQ(Bundle::State::STATE_INSTALLED, bundle.GetState());
  int tc0 = bt->GetTrackingCount();

  // If Remove removes a bundle from being tracked, tracking count increments by 1
  bt->Remove(bundle);
  ASSERT_EQ(Bundle::State::STATE_INSTALLED, bundle.GetState());
  int tc1 = bt->GetTrackingCount();

  EXPECT_EQ(1, tc1 - tc0);

  // If Remove doesn't remove a bundle from being tracked, tracking count stays the same
  bt->Remove(bundle);
  int tc2 = bt->GetTrackingCount();

  EXPECT_EQ(0, tc2 - tc1);
  bt->Close();
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
