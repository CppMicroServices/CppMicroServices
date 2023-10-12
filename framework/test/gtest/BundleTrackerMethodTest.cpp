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
#    undef GetObject
#endif

using namespace cppmicroservices;

class BundleTrackerMethodTest : public ::testing::Test
{
  protected:
    Framework framework;
    BundleContext context;
    BundleTracker<>::BundleStateMaskType all_states
        = BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE,
                                           Bundle::State::STATE_INSTALLED,
                                           Bundle::State::STATE_RESOLVED,
                                           Bundle::State::STATE_STARTING,
                                           Bundle::State::STATE_STOPPING,
                                           Bundle::State::STATE_UNINSTALLED);

  public:
    BundleTrackerMethodTest() : framework(FrameworkFactory().NewFramework()) {};

    ~BundleTrackerMethodTest() override = default;

    void
    SetUp() override
    {
        framework.Start();
        context = framework.GetBundleContext();
    }

    void
    TearDown() override
    {
        framework.Stop();
        framework.WaitForStop(std::chrono::milliseconds::zero());
    }
};

TEST_F(BundleTrackerMethodTest, TestCanCreateTracker)
{
    ASSERT_NO_THROW(BundleTracker bundleTracker(context, all_states)) << "Creation of BundleTracker failed";
}

TEST_F(BundleTrackerMethodTest, TestGetBundles)
{
    BundleTracker<> bundleTracker(context, all_states);
    std::vector<Bundle> bundles = bundleTracker.GetBundles();
    EXPECT_EQ(0, bundles.size()) << "GetBundles() should return an empty vector before Open()";

    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
    Bundle bundleMain = cppmicroservices::testing::GetBundle("main", context);
    Bundle bundleSys = cppmicroservices::testing::GetBundle("system_bundle", context);
    Bundle bundleA = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    bundles = bundleTracker.GetBundles();

    bool mainTracked = std::find(bundles.begin(), bundles.end(), bundleMain) != bundles.end();
    bool sysTracked = std::find(bundles.begin(), bundles.end(), bundleSys) != bundles.end();
    bool bundleATracked = std::find(bundles.begin(), bundles.end(), bundleA) != bundles.end();

    EXPECT_TRUE(mainTracked) << "GetBundles() should include the main bundle";
    EXPECT_TRUE(sysTracked) << "GetBundles() should include the system bundle";
    EXPECT_TRUE(bundleATracked) << "GetBundles() should include the test bundle";

    bundleTracker.Close();
    bundles = bundleTracker.GetBundles();
    EXPECT_EQ(0, bundles.size()) << "GetBundles() should return an empty vector after Close()";
}

TEST_F(BundleTrackerMethodTest, TestGetObject)
{
    // Given an open BundleTracker not tracking custom objects
    BundleTracker<> bundleTracker(context, all_states);
    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";

    // When a bundle is tracked
    Bundle bundleA = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    auto bundles = bundleTracker.GetBundles();
    bool bundleATracked = std::find(bundles.begin(), bundles.end(), bundleA) != bundles.end();
    ASSERT_TRUE(bundleATracked) << "A bundle that should be tracked is untracked";

    // Then GetObject should return the bundle
    EXPECT_EQ(bundleA, bundleTracker.GetObject(bundleA))
        << "GetObject for a tracked bundle should return the bundle when no custom "
           "objects were used";

    // When the bundle is untracked, then GetObject should return nullopt
    bundleTracker.Remove(bundleA);
    EXPECT_EQ(std::nullopt, bundleTracker.GetObject(bundleA))
        << "GetObject for an untracked, valid bundle should return nullopt";

    // When the bundle is invalid, then GetObject should return nullopt
    Bundle invalidBundle = Bundle();
    ASSERT_NO_THROW(bundleTracker.GetObject(invalidBundle)) << "GetObject threw an error for an invalid bundle";
    EXPECT_EQ(std::nullopt, bundleTracker.GetObject(invalidBundle))
        << "GetObject for an untracked, invalid bundle should return nullopt";

    bundleTracker.Close();
}

TEST_F(BundleTrackerMethodTest, TestGetTracked)
{
    BundleTracker<> bundleTracker(context, all_states);
    auto tracked = bundleTracker.GetTracked();
    EXPECT_EQ(0, tracked.size()) << "GetTracked() should return an empty map before Open()";

    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
    Bundle bundleMain = cppmicroservices::testing::GetBundle("main", context);
    Bundle bundleSys = cppmicroservices::testing::GetBundle("system_bundle", context);
    Bundle bundleA = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    tracked = bundleTracker.GetTracked();

    bool mainTracked = tracked.find(bundleMain) != tracked.end();
    bool sysTracked = tracked.find(bundleSys) != tracked.end();
    bool bundleATracked = tracked.find(bundleA) != tracked.end();

    EXPECT_TRUE(mainTracked) << "GetTracked() should include the main bundle";
    EXPECT_TRUE(sysTracked) << "GetTracked() should include the system bundle";
    EXPECT_TRUE(bundleATracked) << "GetTracked() should include the test bundle";

    bundleTracker.Close();
    tracked = bundleTracker.GetTracked();
    EXPECT_EQ(0, tracked.size()) << "GetTracked() should return an empty map after Close()";
}

TEST_F(BundleTrackerMethodTest, TestIsEmpty)
{
    BundleTracker<> bundleTracker(context, all_states);
    EXPECT_TRUE(bundleTracker.IsEmpty()) << "Unopened BundleTracker should be empty";

    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
    Bundle bundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    ASSERT_EQ(bundle, bundleTracker.GetObject(bundle)) << "BundleTracker failed to track a bundle";
    EXPECT_FALSE(bundleTracker.IsEmpty()) << "BundleTracker should not be empty when a bundle is tracked";

    bundleTracker.Close();
    EXPECT_TRUE(bundleTracker.IsEmpty()) << "Closed BundleTracker should be empty";
}

TEST_F(BundleTrackerMethodTest, TestSize)
{
    auto stateMask = BundleTracker<>::CreateStateMask(Bundle::State::STATE_UNINSTALLED);
    BundleTracker<> bundleTracker(context, stateMask);
    EXPECT_EQ(0, bundleTracker.Size()) << "Size of unopened BundleTracker was not 0";

    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
    EXPECT_EQ(0, bundleTracker.Size()) << "Size should be 0 when no bundles has entered a tracked state";

    Bundle bundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    bundle.Uninstall();
    EXPECT_EQ(1, bundleTracker.Size()) << "Size should be 1 when 1 bundle entered a tracked state";

    bundleTracker.Close();
    EXPECT_EQ(0, bundleTracker.Size()) << "Size of closed BundleTracker was not 0";
}

TEST_F(BundleTrackerMethodTest, TestGetTrackingCountWorksAfterBundle)
{
    auto stateMask = BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE);
    BundleTracker<> bundleTracker(context, stateMask);
    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";

    int preCount = bundleTracker.GetTrackingCount();
    Bundle bundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    bundle.Start(); // bundle: installed->resolved->starting->ACTIVE
    int postCount = bundleTracker.GetTrackingCount();

    EXPECT_EQ(1, postCount - preCount) << "Tracking count didn't increment by 1 after a bundle was added";
    bundleTracker.Close();
}

TEST_F(BundleTrackerMethodTest, TestGetTrackingCountWorksAfterBundleModified)
{
    BundleTracker<> bundleTracker(context, all_states);
    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
    Bundle bundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    ASSERT_EQ(Bundle::State::STATE_INSTALLED, bundle.GetState()) << "Test bundle failed to install";

    int preCount = bundleTracker.GetTrackingCount();
    bundle.Start(); // bundle: INSTALLED->RESOLVED->STARTING->ACTIVE
    ASSERT_EQ(Bundle::State::STATE_ACTIVE, bundle.GetState()) << "Test bundle failed to start";
    int postCount = bundleTracker.GetTrackingCount();

    EXPECT_EQ(3, postCount - preCount) << "Tracking count didn't increment by 3 after a bundle was modified 3 "
                                          "times";
    bundleTracker.Close();
}

TEST_F(BundleTrackerMethodTest, TestGetTrackingCountWorksAfterBundleRemovedByStateChange)
{
    auto stateMask = BundleTracker<>::CreateStateMask(Bundle::State::STATE_INSTALLED);
    BundleTracker<> bundleTracker(context, stateMask);
    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
    Bundle bundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    ASSERT_EQ(Bundle::State::STATE_INSTALLED, bundle.GetState()) << "Test bundle failed to install";

    int preCount = bundleTracker.GetTrackingCount();
    bundle.Start(); // bundle: INSTALLED->resolved->starting->active
    ASSERT_EQ(Bundle::State::STATE_ACTIVE, bundle.GetState()) << "Test bundle failed to start";
    int postCount = bundleTracker.GetTrackingCount();

    EXPECT_EQ(1, postCount - preCount) << "Tracking count didn't increment by 1 after a bundle was removed";
    bundleTracker.Close();
}

TEST_F(BundleTrackerMethodTest, TestGetTrackingCountWorksAfterRemoveMethod)
{
    BundleTracker<> bundleTracker(context, all_states);
    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
    Bundle bundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    ASSERT_EQ(Bundle::State::STATE_INSTALLED, bundle.GetState()) << "Test bundle failed to install";

    // If Remove removes a bundle from being tracked, tracking count increments by 1
    int preCount = bundleTracker.GetTrackingCount();
    bundleTracker.Remove(bundle);
    ASSERT_EQ(Bundle::State::STATE_INSTALLED, bundle.GetState()) << "Remove() altered the bundle being removed";
    int postCount = bundleTracker.GetTrackingCount();

    EXPECT_EQ(1, postCount - preCount) << "Tracking count didn't increment by 1 after Remove() removed a bundle";

    // If Remove doesn't remove a bundle from being tracked, tracking count stays the same
    preCount = postCount;
    bundleTracker.Remove(bundle);
    postCount = bundleTracker.GetTrackingCount();

    EXPECT_EQ(0, postCount - preCount) << "Tracking count changed after Remove() didn't remove a bundle";
    bundleTracker.Close();
}

TEST_F(BundleTrackerMethodTest, TestGetTrackingCountWorksWhenClosed)
{
    BundleTracker<> bundleTracker(context, all_states);
    EXPECT_EQ(-1, bundleTracker.GetTrackingCount()) << "Tracking count of unopened BundleTracker was not -1";

    bundleTracker.Open();
    bundleTracker.Close();

    EXPECT_EQ(-1, bundleTracker.GetTrackingCount()) << "Tracking count of unopened BundleTracker was not -1";
}

TEST_F(BundleTrackerMethodTest, TestRemove)
{
    // Given a BundleTracker tracking a bundle
    BundleTracker<> bundleTracker(context, all_states);
    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
    Bundle bundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    auto bundlesTracked = bundleTracker.GetBundles();
    bool bundleTracked = std::find(bundlesTracked.begin(), bundlesTracked.end(), bundle) != bundlesTracked.end();
    ASSERT_TRUE(bundleTracked) << "BundleTracker failed to track a bundle";

    // When Remove is called
    bundleTracker.Remove(bundle);

    // Then the bundle should not be tracked
    bundlesTracked = bundleTracker.GetBundles();
    bool bundleStillTracked = std::find(bundlesTracked.begin(), bundlesTracked.end(), bundle) != bundlesTracked.end();
    EXPECT_FALSE(bundleStillTracked) << "Calling Remove() didn't removed tracked bundle from being tracked";

    bundleTracker.Close();
}

TEST_F(BundleTrackerMethodTest, TestBundleRemovedByRemoveTrackable)
{
    BundleTracker<> bundleTracker(context, all_states);
    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
    Bundle testBundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");

    bundleTracker.Remove(testBundle);
    testBundle.Start();

    bool testBundleTracked = bundleTracker.GetObject(testBundle) == testBundle;
    EXPECT_TRUE(testBundleTracked) << "Remove() was called on a tracked bundle, then later entered a tracked "
                                      "state. The bundle should be tracked but wasn't";
}

TEST_F(BundleTrackerMethodTest, TestDefaultAddingBundleReturnsCorrectValue)
{
    BundleTracker<> bundleTracker(context, all_states);
    Bundle bundle = Bundle();
    EXPECT_EQ(bundle, bundleTracker.AddingBundle(bundle, BundleEvent()))
        << "Default AddingBundle should return the inputted bundle";
}

TEST_F(BundleTrackerMethodTest, TestRemoveUntrackedBundleDoesNothing)
{
    auto stateMask = BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE);
    BundleTracker<> bundleTracker(context, stateMask);
    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
    Bundle bundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");

    int preCount = bundleTracker.GetTrackingCount();
    ASSERT_NO_THROW(bundleTracker.Remove(bundle)) << "Manually removing untracked Bundle should not throw";
    int postCount = bundleTracker.GetTrackingCount();
    EXPECT_EQ(preCount, postCount) << "BundleTracker should not change the tracking count when untracked "
                                      "Bundle is manually removed";

    bundleTracker.Close();
}

TEST_F(BundleTrackerMethodTest, TestReopeningTracker)
{
    auto stateMask = BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE);
    BundleTracker<> bundleTracker(context, stateMask);
    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
    Bundle bundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    bundle.Start();
    ASSERT_EQ(2, bundleTracker.Size()) << "Bundles tracked incorrectly";
    bundleTracker.Close();

    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start after it was previously opened and "
                                             "closed";
    EXPECT_EQ(2, bundleTracker.Size()) << "BundleTracker is tracking bundles incorrectly after being reopened";

    bundleTracker.Close();
}

TEST_F(BundleTrackerMethodTest, TestBundlesInUntrackedStatesUntracked)
{
    auto stateMask = BundleTracker<>::CreateStateMask(Bundle::State::STATE_STARTING, Bundle::State::STATE_STOPPING);
    BundleTracker<> bundleTracker(context, stateMask);
    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";

    Bundle bundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    bundle.Start();

    EXPECT_EQ(0, bundleTracker.Size()) << "No bundles should be tracked when none are in a state covered by the "
                                          "state mask";
    bundleTracker.Close();
}

TEST_F(BundleTrackerMethodTest, TestOpenWithInvalidContextThrowsError)
{
    auto invalidContext = BundleContext();
    BundleTracker<> bundleTracker(invalidContext, all_states);

    EXPECT_THROW(bundleTracker.Open(), std::runtime_error) << "Opening BundleTracker with invalid context should throw";
}

TEST_F(BundleTrackerMethodTest, TestOpeningOpenTrackerDoesNothing)
{
    BundleTracker<> bundleTracker(context, all_states);
    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
    Bundle bundle = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    bundle.Start();

    int preCount = bundleTracker.GetTrackingCount();
    int preSize = bundleTracker.Size();
    ASSERT_NO_THROW(bundleTracker.Open()) << "Open() on opened BundleTracker threw an error";
    int postCount = bundleTracker.GetTrackingCount();
    int postSize = bundleTracker.Size();

    EXPECT_EQ(0, postCount - preCount) << "Open() on opened BundleTracker increased the tracking count";
    EXPECT_EQ(0, postSize - preSize) << "Open() on opened BundleTracker changed the size";
    bundleTracker.Close();
}

TEST_F(BundleTrackerMethodTest, TestClosingClosedTrackerDoesNothing)
{
    BundleTracker<> bundleTracker(context, all_states);
    ASSERT_NO_THROW(bundleTracker.Open()) << "BundleTracker failed to start";
    ASSERT_NO_THROW(bundleTracker.Close()) << "BundleTracker failed to close";
    int preCount = bundleTracker.GetTrackingCount();

    ASSERT_NO_THROW(bundleTracker.Close()) << "Closing closed BundleTracker should no-op";
    int postCount = bundleTracker.GetTrackingCount();
    EXPECT_EQ(preCount, postCount) << "Closing closed BundleTracker should not change the tracking count";
}

TEST_F(BundleTrackerMethodTest, TestClosingUnopenTrackerDoesNothing)
{
    BundleTracker<> bundleTracker(context, all_states);
    int preCount = bundleTracker.GetTrackingCount();

    ASSERT_NO_THROW(bundleTracker.Close()) << "Closing unopened BundleTracker should no-op";
    int postCount = bundleTracker.GetTrackingCount();
    EXPECT_EQ(preCount, postCount) << "Closing unopened BundleTracker should not change the tracking count";
}
