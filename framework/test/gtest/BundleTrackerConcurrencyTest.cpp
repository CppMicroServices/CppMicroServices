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

#include <future>
#include <optional>
#include <unordered_set>

#include "TestUtils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace cppmicroservices;

class BundleTrackerConcurrencyTest : public ::testing::Test
{
  protected:
    Framework framework;
    BundleContext context;
    static constexpr BundleTracker<>::BundleStateMaskType all_states
        = BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE,
                                           Bundle::State::STATE_INSTALLED,
                                           Bundle::State::STATE_RESOLVED,
                                           Bundle::State::STATE_STARTING,
                                           Bundle::State::STATE_STOPPING,
                                           Bundle::State::STATE_UNINSTALLED);

  public:
    BundleTrackerConcurrencyTest() : framework(FrameworkFactory().NewFramework()) {};

    ~BundleTrackerConcurrencyTest() override = default;

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

MATCHER_P(HasName, name, "") { return arg.GetSymbolicName() == name; }

#ifdef US_ENABLE_THREADING_SUPPORT
class MockCustomizer2 : public BundleTrackerCustomizer<>
{
  public:
    MOCK_METHOD(std::optional<Bundle>, AddingBundle, (Bundle const&, BundleEvent const&), (override));
    MOCK_METHOD(void, ModifiedBundle, (Bundle const&, BundleEvent const&, Bundle const&), (override));
    MOCK_METHOD(void, RemovedBundle, (Bundle const&, BundleEvent const&, Bundle const&), (override));
};

TEST_F(BundleTrackerConcurrencyTest, TestConcurrentOpenClose)
{
    auto bundleTracker = std::make_unique<BundleTracker<>>(context, all_states);

    size_t numThreads = std::thread::hardware_concurrency();
    ASSERT_GT(numThreads, 0ull) << "number of threads is 0";
    std::vector<std::future<void>> futures;
    std::promise<void> gate;
    auto gateFuture = gate.get_future().share();
    for (size_t i = 0; i <= numThreads; ++i)
    {
        futures.push_back(std::async(std::launch::async,
                                     [i, &bundleTracker, &gateFuture]()
                                     {
                                         gateFuture.get();
                                         for (int n = 0; n < 1000; ++n)
                                         {
                                             if (i % 2 == 0)
                                             {
                                                 bundleTracker->Open();
                                             }
                                             else
                                             {
                                                 bundleTracker->Close();
                                             }
                                         }
                                     }));
    }

    gate.set_value();

    for (auto& asyncFuture : futures)
    {
        asyncFuture.get();
    }
}

TEST_F(BundleTrackerConcurrencyTest, TestOpeningTrackerWhileBundlesChange)
{
    auto customizer = std::make_shared<MockCustomizer2>();
    auto stateMask = BundleTracker<>::CreateStateMask(Bundle::State::STATE_ACTIVE);
    auto bundleTracker = std::make_unique<BundleTracker<>>(context, stateMask, customizer);

    size_t numThreads = std::thread::hardware_concurrency();
    ASSERT_GT(numThreads, 0ull) << "number of threads is 0";
    std::vector<std::future<void>> futures;
    std::promise<void> gate;
    auto gateFuture = gate.get_future().share();

    auto openTrackerFuture = std::async(std::launch::async,
                                        [&bundleTracker, &gateFuture]()
                                        {
                                            gateFuture.get();
                                            bundleTracker->Open();
                                        });

    auto startBundlesFuture
        = std::async(std::launch::async,
                     [this, &gateFuture]()
                     {
                         gateFuture.get();
                         cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA").Start();
                         cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA2").Start();
                         cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleB").Start();
                         cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleH").Start();
                         cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleM").Start();
                         cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleR").Start();
                     });

    std::unordered_set<std::string> trackInitial {};
    std::unordered_set<std::string> rcvdEvt {};

    // can receive anywhere between 7-13 evts.
    // 7 calls from trackInitial (one for each bundle + 1 for system bundle)
    // 6 calls from bundleEvts (1 for each bundle.
    //     - If these events are fired after the initial bundles to be tracked are set but don't run till after the
    //     trackInitial calls are made
    //     - we will first see the AddingBundle call with a null evt and then a real bundle event.
    //     - It cannot be the other way around (bundleEvent and then the trackInitial))
    ON_CALL(*customizer, AddingBundle(::testing::_, ::testing::_))
        .WillByDefault(
            [&trackInitial, &rcvdEvt](Bundle const& bundle, BundleEvent const& evt)
            {
                auto name = bundle.GetSymbolicName();
                if (!evt)
                {
                    // from trackInitial
                    EXPECT_TRUE(trackInitial.count(name) == 0);
                    EXPECT_TRUE(rcvdEvt.count(name) == 0);
                    trackInitial.insert(name);
                    return std::optional<Bundle> {};
                };
                // shouldn't get more than one evt per bundle
                EXPECT_TRUE(rcvdEvt.count(name) == 0);

                rcvdEvt.insert(name);
                return std::optional<Bundle> {};
            });

    gate.set_value();

    openTrackerFuture.get();
    startBundlesFuture.get();

    bundleTracker->Close();
}

TEST_F(BundleTrackerConcurrencyTest, TestNoRaceConditionForRemovingChangingBundle)
{
    auto customizer = std::make_shared<MockCustomizer2>();
    auto bundleTracker = std::make_unique<BundleTracker<>>(context, all_states, customizer);
    Bundle bundleA = cppmicroservices::testing::InstallLib(framework.GetBundleContext(), "TestBundleA");
    // make AddingBundle return nullopt for all bundles besides bundleA
    EXPECT_CALL(*customizer, AddingBundle).WillRepeatedly(::testing::Return(std::nullopt));
    EXPECT_CALL(*customizer, AddingBundle(HasName("TestBundleA"), ::testing::_)).WillOnce(::testing::ReturnArg<0>());
    bundleTracker->Open();

    size_t numThreads = std::thread::hardware_concurrency();
    ASSERT_GT(numThreads, 0ull) << "number of threads is 0";
    std::promise<void> gate;
    auto gateFuture = gate.get_future().share();
    auto removeBundleFuture = std::async(std::launch::async,
                                         [&gateFuture, &bundleTracker, &bundleA]()
                                         {
                                             gateFuture.get();
                                             bundleTracker->Remove(bundleA);
                                         });
    auto startBundleFuture = std::async(std::launch::async,
                                        [&gateFuture, &bundleA]()
                                        {
                                            gateFuture.get();
                                            bundleA.Start();
                                        });

    // The Remove can occur while bundleA is in any state while being started,
    // so all 4 of the following series of callbacks are possible:
    //    Remove -> Add -> Modify -> Modify
    //    Modify -> Remove -> Add -> Modify
    //    Modify -> Modify -> Remove -> Add
    //    Modify -> Modify -> Modify -> Remove

    int addModifyCounter = 0; // counts the number of AddingBundle and ModifiedBundle callbacks
    auto increment = [&addModifyCounter]() { addModifyCounter++; };

    EXPECT_CALL(*customizer, AddingBundle)
        .Times(::testing::AtMost(1))
        .WillOnce(::testing::DoAll(::testing::Invoke(increment), ::testing::ReturnArg<0>()));
    EXPECT_CALL(*customizer, ModifiedBundle).Times(::testing::AtLeast(2)).WillRepeatedly(::testing::Invoke(increment));
    EXPECT_CALL(*customizer, RemovedBundle).Times(1);
    gate.set_value();

    removeBundleFuture.get();
    startBundleFuture.get();

    EXPECT_EQ(addModifyCounter, 3) << "The wrong number of callbacks were issued";

    // What happens after is out of scope for this test
    EXPECT_CALL(*customizer, RemovedBundle).Times(::testing::AnyNumber());
    bundleTracker->Close();
}
#endif
