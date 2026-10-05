#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/ListenerToken.h"
#include "cppmicroservices/ServiceEvent.h"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/SharedLibraryException.h"

#include "cppmicroservices/util/FileSystem.h"
#include "cppmicroservices/util/String.h"

#include "FrameworkTestActivator.h"
#include "TestUtilBundleListener.h"
#include "TestUtilFrameworkListener.h"
#include "TestUtils.h"
#include "TestingConfig.h"
#include "TestUtilListenerHelpers.h"

#include "gmock/gmock.h"
#include "cppmicroservices/logservice/LogService.hpp"

#include <chrono>
#include <future>
#include <thread>
#include <iostream>
#include <algorithm>
#include <mutex>

#include "gtest/gtest.h"

US_MSVC_PUSH_DISABLE_WARNING(4996)

using namespace cppmicroservices;
using namespace cppmicroservices::testing;

class BundleLifecycleTest : public ::testing::Test
{
  protected:
    Framework framework;
    BundleContext context;

  public:
    BundleLifecycleTest() : framework(FrameworkFactory().NewFramework()) {};

    ~BundleLifecycleTest() override = default;

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

class MockLogger : public cppmicroservices::logservice::LogService
{
    public:
    MOCK_METHOD2(Log, void(cppmicroservices::logservice::SeverityLevel, std::string const&));
    MOCK_METHOD3(Log,
                    void(cppmicroservices::logservice::SeverityLevel, std::string const&, std::exception_ptr const));
    MOCK_METHOD3(Log,
                    void(cppmicroservices::ServiceReferenceBase const&,
                        cppmicroservices::logservice::SeverityLevel,
                        std::string const&));
    MOCK_METHOD4(Log,
                    void(cppmicroservices::ServiceReferenceBase const&,
                        cppmicroservices::logservice::SeverityLevel,
                        std::string const&,
                        std::exception_ptr const));
    MOCK_CONST_METHOD1(getLogger, std::shared_ptr<cppmicroservices::logservice::Logger>(std::string const&));
    MOCK_CONST_METHOD2(getLogger,
                        std::shared_ptr<cppmicroservices::logservice::Logger>(cppmicroservices::Bundle const&,
                                                                                std::string const&));
};

TEST_F(BundleLifecycleTest, TestExpectedStateTransitions)
{
    // Basic test for final state values and listener events for some of the transitions

    TestBundleListener listener;
    BundleListenerRegistrationHelper<TestBundleListener> listenerReg(
        context, &listener, &TestBundleListener::BundleChanged);
    std::vector<BundleEvent> bundleEvents;

    auto bundleA = InstallLib(context, "TestBundleA");
    ASSERT_TRUE(bundleA);
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_INSTALLED);
#ifdef US_BUILD_SHARED_LIBS
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundleA));
#endif

    bundleA.Stop(); //BPInstalledState::Stop()
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_INSTALLED);

    bundleA.Start(); //BPInstalledState::Start()
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, bundleA));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundleA));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, bundleA));

    bundleA.Stop(); //BPActiveState::Stop()
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_RESOLVED);
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, bundleA));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, bundleA));

    bundleA.Stop(); //BPResolvedState::Stop()
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_RESOLVED);

    bundleA.Start(); //BPResolvedState::Start()
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_ACTIVE);
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundleA));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, bundleA));

    bundleA.Uninstall(); //BPActiveState::Uninstall()
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_UNINSTALLED);
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, bundleA));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, bundleA));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNRESOLVED, bundleA));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNINSTALLED, bundleA));

    ASSERT_TRUE(listener.CheckListenerEvents(bundleEvents, false));

};

TEST_F(BundleLifecycleTest, TestBundleStateDuringListenerEvents)
{
    // Testing for state values when the listener events occur

    auto bundle = InstallLib(context, "TestBundleA");
    ASSERT_TRUE(bundle);
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_INSTALLED);

    std::map<BundleEvent::Type, Bundle::State> expectedStatesByEvent = {
        { BundleEvent::BUNDLE_RESOLVED, Bundle::STATE_RESOLVED },
        { BundleEvent::BUNDLE_STARTING, Bundle::STATE_STARTING },
        { BundleEvent::BUNDLE_STARTED, Bundle::STATE_ACTIVE },
        { BundleEvent::BUNDLE_STOPPING, Bundle::STATE_STOPPING },
        { BundleEvent::BUNDLE_STOPPED, Bundle::STATE_RESOLVED },
        { BundleEvent::BUNDLE_UNRESOLVED, Bundle::STATE_INSTALLED },
        { BundleEvent::BUNDLE_UNINSTALLED, Bundle::STATE_UNINSTALLED }
    };
    std::map<BundleEvent::Type, int> observedEvents;

    auto listener = [&](BundleEvent const& evt)
    {
        if (evt.GetBundle().GetBundleId() != bundle.GetBundleId())
        {
            return;
        }

        auto expectedState = expectedStatesByEvent.find(evt.GetType());
        if (expectedState == expectedStatesByEvent.end())
        {
            return;
        }

        observedEvents[evt.GetType()]++;
        EXPECT_EQ(evt.GetBundle().GetState(), expectedState->second)
            << "Unexpected bundle state during listener event " << evt.GetType();
    };

    auto listenerToken = context.AddBundleListener(listener);

    bundle.Start();
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_ACTIVE);

    bundle.Stop();
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_RESOLVED);

    bundle.Uninstall();
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_UNINSTALLED);

    context.RemoveListener(std::move(listenerToken));

    for (auto const& expectedState : expectedStatesByEvent)
    {
        EXPECT_GT(observedEvents[expectedState.first], 0)
            << "Did not observe expected listener event " << expectedState.first;
    }
}

TEST_F(BundleLifecycleTest, TestConcurrentStartCallsBothObserveActive)
{
    // For several racing Start() calls, they all only return after the winning Start() finishes 
    auto bundle = InstallLib(context, "TestBundleA");
    ASSERT_TRUE(bundle);
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_INSTALLED);

    std::promise<void> go;
    std::shared_future<void> ready(go.get_future());
    constexpr int numCalls = 10;
    std::vector<std::promise<void>> readies(numCalls);
    std::vector<std::future<Bundle::State>> startResults(numCalls);

    for (int i = 0; i < numCalls; ++i)
    {
        startResults[i] = std::async(
            std::launch::async,
            [bundle, ready, &readies, i]() mutable
            {
                readies[i].set_value();
                ready.wait();
                bundle.Start();
                return bundle.GetState();
            });
    }

    for (int i = 0; i < numCalls; ++i)
    {
        readies[i].get_future().wait();
    }

    go.set_value();

    for (auto& startResult : startResults)
    {
        EXPECT_EQ(startResult.get(), Bundle::STATE_ACTIVE);
    }

    ASSERT_EQ(bundle.GetState(), Bundle::STATE_ACTIVE);
}

TEST_F(BundleLifecycleTest, TestConcurrentStopCallsBothObserveResolved)
{
    // For several racing Stop()) calls, they all only return after the winning Stop() finishes 
    auto bundle = InstallLib(context, "TestBundleA");
    ASSERT_TRUE(bundle);

    bundle.Start();
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_ACTIVE);

    std::promise<void> go;
    std::shared_future<void> ready(go.get_future());
    constexpr int numCalls = 10;
    std::vector<std::promise<void>> readies(numCalls);
    std::vector<std::future<Bundle::State>> stopResults(numCalls);

    for (int i = 0; i < numCalls; ++i)
    {
        stopResults[i] = std::async(
            std::launch::async,
            [bundle, ready, &readies, i]() mutable
            {
                readies[i].set_value();
                ready.wait();
                bundle.Stop();
                return bundle.GetState();
            });
    }

    for (int i = 0; i < numCalls; ++i)
    {
        readies[i].get_future().wait();
    }

    go.set_value();

    for (auto& stopResult : stopResults)
    {
        EXPECT_EQ(stopResult.get(), Bundle::STATE_RESOLVED);
    }

    ASSERT_EQ(bundle.GetState(), Bundle::STATE_RESOLVED);
}

TEST_F(BundleLifecycleTest, TestConcurrentUninstallCallsBothObserveUninstalled)
{
    // Testing that for several racing Uninstall() calls, they all only return after the winning Uninstall() finishes. 
    // Since calling Uninstall() on an already Uninstalled bundle throws, we use try-catch blocks to handle the losing calls.
    auto bundle = InstallLib(context, "TestBundleA");
    ASSERT_TRUE(bundle);
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_INSTALLED);

    std::promise<void> go;
    std::shared_future<void> ready(go.get_future());
    constexpr int numCalls = 10;
    std::vector<std::promise<void>> readies(numCalls);
    struct UninstallResult
    {
        bool threw;
        Bundle::State state;
    };
    std::vector<std::future<UninstallResult>> uninstallResults(numCalls);

    for (int i = 0; i < numCalls; ++i)
    {
        uninstallResults[i] = std::async(
            std::launch::async,
            [bundle, ready, &readies, i]() mutable
            {
                readies[i].set_value();
                ready.wait();
                try
                {
                    bundle.Uninstall();
                    return UninstallResult { false, bundle.GetState() };
                }
                catch (...)
                {
                    return UninstallResult { true, bundle.GetState() };
                }
            });
    }

    for (int i = 0; i < numCalls; ++i)
    {
        readies[i].get_future().wait();
    }

    go.set_value();

    int successfulCalls = 0;
    int throwingCalls = 0;
    for (auto& uninstallResult : uninstallResults)
    {
        auto result = uninstallResult.get();
        if (result.threw)
        {
            ++throwingCalls;
        }
        else
        {
            ++successfulCalls;
        }
        EXPECT_EQ(result.state, Bundle::STATE_UNINSTALLED);
    }

    EXPECT_GE(successfulCalls, 1);
    EXPECT_GE(throwingCalls, 0);
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_UNINSTALLED);
}

TEST_F(BundleLifecycleTest, TestStartStopDroppedTransitions)
{
    // Execute Stop() and Start() concurrently over multiple iterations
    // Check the possible end states.
    std::map<Bundle::State, int> observed;
    int iterations = 100;

    std::vector<Bundle::State> expectedStates = {
      Bundle::STATE_ACTIVE,
      Bundle::STATE_RESOLVED,
      Bundle::STATE_INSTALLED
    };

    for(int i = 0; i < iterations; ++i){

        Framework iterFramework = FrameworkFactory().NewFramework();
        iterFramework.Start();
        auto iterContext = iterFramework.GetBundleContext();

        auto bundleA = InstallLib(iterContext, "TestBundleA");
        ASSERT_TRUE(bundleA);

        ASSERT_EQ(bundleA.GetState(), Bundle::STATE_INSTALLED);

        std::promise<void> go;
        std::shared_future<void> ready(go.get_future());
        constexpr int numCalls = 2;
        std::vector<std::promise<void>> readies(numCalls); //vector of promises to tell the first thread when the bundles are all prepared
        std::vector<std::future<void>> bundleStateChanges(numCalls);

        for (int i = 0; i < numCalls; ++i)
        {
            bundleStateChanges[i] = std::async(
                std::launch::async,
                [bundleA, ready, &readies, i]() mutable
                {
                    readies[i].set_value();
                    ready.wait();
                    ((i % 2) ? bundleA.Start() : bundleA.Stop());
                });
        }

        for (int i = 0; i < numCalls; ++i)
        {
            readies[i].get_future().wait();
        }

        go.set_value();

        for (auto& bundleStateChange : bundleStateChanges)
        {
            bundleStateChange.wait();
        }

        auto state = bundleA.GetState();

        ASSERT_TRUE(std::find(expectedStates.begin(), expectedStates.end(), state) != expectedStates.end())
        << "Unexpected final bundle state: " << state
        << " on iteration " << i;

        observed[state]++;

        iterFramework.Stop();
        iterFramework.WaitForStop(std::chrono::milliseconds::zero());
    }

    for (auto expectedState : expectedStates)
    {
        if (observed[expectedState] == 0)
        {
            GTEST_LOG_(WARNING) << "Did not observe expected final state "
                                << expectedState
                                << " after " << iterations << " iterations";
        }
    }
}

TEST_F(BundleLifecycleTest, TestUninstallDroppedTransitions)
{
    // Execute Stop(), Start(), and Uninstall() concurrently. 
    // Check that the final state is Uninstalled. 

    std::map<Bundle::State, int> observed;
    int iterations = 100;

    std::vector<Bundle::State> expectedStates = {
      Bundle::STATE_UNINSTALLED
    };

    for(int i = 0; i < iterations; ++i){

        Framework iterFramework = FrameworkFactory().NewFramework();
        iterFramework.Start();
        auto iterContext = iterFramework.GetBundleContext();

        auto bundleA = InstallLib(iterContext, "TestBundleA");
        ASSERT_TRUE(bundleA);

        ASSERT_EQ(bundleA.GetState(), Bundle::STATE_INSTALLED);

        std::promise<void> go;
        std::shared_future<void> ready(go.get_future());
        constexpr int numCalls = 3;
        std::vector<std::promise<void>> readies(numCalls);
        std::vector<std::future<void>> bundleStateChanges(numCalls);

        for (int i = 0; i < numCalls; ++i)
        {
            bundleStateChanges[i] = std::async(
                std::launch::async,
                [bundleA, ready, &readies, i]() mutable
                {
                    readies[i].set_value();
                    ready.wait();

                    if(i == 0){
                        bundleA.Start();
                    } else if(i == 1){
                        bundleA.Stop();
                    } else {
                        bundleA.Uninstall();
                    }   
                });
        }

        for (int i = 0; i < numCalls; ++i)
        {
            readies[i].get_future().wait();
        }

        go.set_value();

        for (auto& bundleStateChange : bundleStateChanges)
        {
            bundleStateChange.wait();
        }

        auto state = bundleA.GetState();

        ASSERT_TRUE(std::find(expectedStates.begin(), expectedStates.end(), state) != expectedStates.end())
        << "Unexpected final bundle state: " << state
        << " on iteration " << i;

        observed[state]++;

        iterFramework.Stop();
        iterFramework.WaitForStop(std::chrono::milliseconds::zero());

    }

    for (auto expectedState : expectedStates)
    {
        if (observed[expectedState] == 0)
        {
            GTEST_LOG_(WARNING) << "Did not observe expected final state "
                                << expectedState
                                << " after " << iterations << " iterations";
        }
    }

}

TEST_F(BundleLifecycleTest, TestBundleMissingDestroyActivator)
{
    // Test that a missing bundle destroy activator results in a throw during a Start() call. 
    // This test is unrelated to the new bundle lifecycle implementation; this was a gap in the old testing suite. 
    auto bundle = cppmicroservices::testing::InstallLib(context, "TestBundleMissingDestroyActivator");

    EXPECT_THROW(
        bundle.Start(),
        std::runtime_error);
}

TEST_F(BundleLifecycleTest, TestStartedBundleListenerThrowsSecurityException)
{
    // Throw a security exception in a BUNDLE_STARTED listener event reponse
    // This should force the bundle back to STATE_RESOLVED via the StartFailed() path

    auto bundle = InstallLib(context, "TestBundleA");

    auto listener = [&](BundleEvent const& evt)
    {
        if (evt.GetType() == BundleEvent::BUNDLE_STARTED
            && evt.GetBundle().GetBundleId() == bundle.GetBundleId())
        {
            throw cppmicroservices::SecurityException("test security exception", evt.GetBundle());
        }
    };

    auto listenerToken = context.AddBundleListener(listener);

    EXPECT_THROW(bundle.Start(), cppmicroservices::SecurityException);

    ASSERT_EQ(bundle.GetState(), Bundle::STATE_RESOLVED);

    context.RemoveListener(std::move(listenerToken));
}

TEST_F(BundleLifecycleTest, TestStartingBundleListenerStopsBundle)
{

    // If Stop is called while the Bundle is in the middle of Starting
    // (We do this via a listener event in this test) 
    // Then we guarentee the final state to be Resolved.

    auto bundle = InstallLib(context, "TestBundleA");

    bool sawStartingEvent = false;
    bool stopScheduledFromListener = false;
    std::future<void> stopAttempt;

    auto listener = [&](BundleEvent const& evt)
    {
        if (evt.GetBundle().GetBundleId() != bundle.GetBundleId())
        {
            return;
        }

        if (evt.GetType() == BundleEvent::BUNDLE_STARTING && !stopScheduledFromListener)
        {
            sawStartingEvent = true;
            stopScheduledFromListener = true;

            stopAttempt = std::async(
                std::launch::async,
                [bundle]() mutable
                {
                    bundle.Stop();
                });
        }
    };

    auto listenerToken = context.AddBundleListener(listener);

    EXPECT_NO_THROW(bundle.Start());

    context.RemoveListener(std::move(listenerToken));

    ASSERT_TRUE(sawStartingEvent);
    ASSERT_TRUE(stopScheduledFromListener);
    ASSERT_TRUE(stopAttempt.valid());

    EXPECT_NO_THROW(stopAttempt.get());

    ASSERT_EQ(bundle.GetState(), Bundle::STATE_RESOLVED);
}

TEST_F(BundleLifecycleTest, TestStoppingBundleListenerStartsBundle)
{
    // If Start is called while the Bundle is in the middle of Stop
    // (We do this via a listener event in this test) 
    // Then we guarentee the final state to be Active.

    auto bundle = InstallLib(context, "TestBundleA");

    bool sawStartingEvent = false;
    bool stopScheduledFromListener = false;
    std::future<void> stopAttempt;

    auto listener = [&](BundleEvent const& evt)
    {
        if (evt.GetBundle().GetBundleId() != bundle.GetBundleId())
        {
            return;
        }

        if (evt.GetType() == BundleEvent::BUNDLE_STOPPING && !stopScheduledFromListener)
        {
            sawStartingEvent = true;
            stopScheduledFromListener = true;

            stopAttempt = std::async(
                std::launch::async,
                [bundle]() mutable
                {
                    bundle.Start();
                });
        }
    };

    auto listenerToken = context.AddBundleListener(listener);

    EXPECT_NO_THROW(bundle.Start());
    EXPECT_NO_THROW(bundle.Stop());

    context.RemoveListener(std::move(listenerToken));

    ASSERT_TRUE(sawStartingEvent);
    ASSERT_TRUE(stopScheduledFromListener);
    ASSERT_TRUE(stopAttempt.valid());

    EXPECT_NO_THROW(stopAttempt.get());

    ASSERT_EQ(bundle.GetState(), Bundle::STATE_ACTIVE);
}

TEST_F(BundleLifecycleTest, TestResolvedBundleListenerStartsBundle)
{
    // If Stop is called while the Bundle is in the middle of Resolving in Start()
    // (We do this via a listener event in this test) 
    // Then we guarentee the final state to be Resolved.

    auto bundle = InstallLib(context, "TestBundleA");

    bool sawStartingEvent = false;
    bool stopScheduledFromListener = false;
    std::future<void> stopAttempt;

    auto listener = [&](BundleEvent const& evt)
    {
        if (evt.GetBundle().GetBundleId() != bundle.GetBundleId())
        {
            return;
        }

        if (evt.GetType() == BundleEvent::BUNDLE_RESOLVED && !stopScheduledFromListener)
        {
            sawStartingEvent = true;
            stopScheduledFromListener = true;

            stopAttempt = std::async(
                std::launch::async,
                [bundle]() mutable
                {
                    bundle.Stop();
                });
        }
    };

    auto listenerToken = context.AddBundleListener(listener);

    EXPECT_NO_THROW(bundle.Start());

    context.RemoveListener(std::move(listenerToken));

    ASSERT_TRUE(sawStartingEvent);
    ASSERT_TRUE(stopScheduledFromListener);
    ASSERT_TRUE(stopAttempt.valid());

    EXPECT_NO_THROW(stopAttempt.get());

    ASSERT_EQ(bundle.GetState(), Bundle::STATE_RESOLVED);
}

TEST_F(BundleLifecycleTest, TestStartFailedRaceWithStart)
{
    // We intentionally fail the first Start() call with a SecurityException.
    // We intentionally call a second Start() that we setup so it won't fail like the first call.
    // The second Start() call will race with the SecurityException recovery.
    // Regardless of if the second Start() succeeds (which is sporadic), the SecurityException recovery should never be interrupted.
    // We check that the Security Exception recovery succeeded via a BundleEvent::BUNDLE_STOPPED listener event.

      auto bundle = InstallLib(context, "TestBundleA");

      std::mutex eventsMutex;
      std::vector<BundleEvent> observedEvents;

      std::promise<void> secondStartAttemptStartedPromise;
      auto secondStartAttemptStarted = secondStartAttemptStartedPromise.get_future();

      std::future<void> secondStartAttempt;
      bool firstStartCall = true;

      auto listener = [&](BundleEvent const& evt)
      {
          if (evt.GetBundle().GetBundleId() != bundle.GetBundleId())
          {
              return;
          }

          {
              std::lock_guard<std::mutex> lock(eventsMutex);
              observedEvents.push_back(evt);
          }

          if (evt.GetType() == BundleEvent::BUNDLE_STARTED && firstStartCall)
          {
              firstStartCall = false;

              secondStartAttempt = std::async(
                  std::launch::async,
                  [&bundle, &secondStartAttemptStartedPromise]() mutable
                  {
                      secondStartAttemptStartedPromise.set_value();
                      bundle.Start();
                  });

              secondStartAttemptStarted.wait();

              throw cppmicroservices::SecurityException("test security exception", evt.GetBundle());
          }
      };

      auto listenerToken = context.AddBundleListener(listener);

      EXPECT_THROW(bundle.Start(), cppmicroservices::SecurityException);

      ASSERT_TRUE(secondStartAttempt.valid());
      EXPECT_NO_THROW(secondStartAttempt.get());

      context.RemoveListener(std::move(listenerToken));

      bool sawStoppedEvent = false;
      {
          std::lock_guard<std::mutex> lock(eventsMutex);
          sawStoppedEvent = std::any_of(
              observedEvents.begin(),
              observedEvents.end(),
              [](BundleEvent const& evt)
              {
                  return evt.GetType() == BundleEvent::BUNDLE_STOPPED;
              });
      }

      EXPECT_TRUE(sawStoppedEvent);

}
