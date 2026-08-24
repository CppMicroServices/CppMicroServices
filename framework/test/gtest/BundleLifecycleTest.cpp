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

    bundleA.Start(); //BPInstalledState::Start(), BPResolvedState::Start(), BPStartingState::Start()
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_ACTIVE);
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, bundleA));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundleA));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, bundleA));

    bundleA.Stop(); //BPActiveState::Stop(), BPStoppingState::Stop()
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_RESOLVED);
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, bundleA));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, bundleA));

    bundleA.Stop(); //BPResolvedState::Stop()
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_RESOLVED);

    bundleA.Start(); //BPResolvedState::Start(), BPStartingState::Start()
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_ACTIVE);
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundleA));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, bundleA));

    bundleA.Uninstall(); //BPActiveState::Uninstall(), BPStoppingState::Uninstall(), BPResolvedState::Uninstall(), BPInstalledState::Uninstall()
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_UNINSTALLED);
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, bundleA));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, bundleA));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNRESOLVED, bundleA));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNINSTALLED, bundleA));

    ASSERT_TRUE(listener.CheckListenerEvents(bundleEvents, false));

};

TEST_F(BundleLifecycleTest, TestBundleActivatorTransition)
{
    // Bundle TestBundleActivatorTransition1 calls Stop on itself in its Bundle Activation Start function
    auto bundleActivatorTransition1 = InstallLib(context, "TestBundleActivatorTransition1");
    ASSERT_EQ(bundleActivatorTransition1.GetState(), Bundle::STATE_INSTALLED);

    bundleActivatorTransition1.Start();
    ASSERT_EQ(bundleActivatorTransition1.GetState(), Bundle::STATE_RESOLVED);

    // Bundle TestBundleActivatorTransition2 calls Start on itself in its Bundle Activation Stop function
    auto bundleActivatorTransition2 = InstallLib(context, "TestBundleActivatorTransition2");
    ASSERT_EQ(bundleActivatorTransition2.GetState(), Bundle::STATE_INSTALLED);

    bundleActivatorTransition2.Start();
    ASSERT_EQ(bundleActivatorTransition2.GetState(), Bundle::STATE_ACTIVE);

    EXPECT_THROW(bundleActivatorTransition2.Stop(), std::runtime_error);
    ASSERT_EQ(bundleActivatorTransition2.GetState(), Bundle::STATE_RESOLVED);

    // Bundle TestBundleActivatorTransition3 calls Uninstall on itself in its Bundle Activation Start function
    auto bundleActivatorTransition3 = InstallLib(context, "TestBundleActivatorTransition3");
    ASSERT_EQ(bundleActivatorTransition3.GetState(), Bundle::STATE_INSTALLED);

    bundleActivatorTransition3.Start();
    ASSERT_EQ(bundleActivatorTransition3.GetState(), Bundle::STATE_UNINSTALLED);

    // Bundle TestBundleActivatorTransition4 calls Uninstall on itself in its Bundle Activation Stop function
    auto bundleActivatorTransition4 = InstallLib(context, "TestBundleActivatorTransition4");
    ASSERT_EQ(bundleActivatorTransition4.GetState(), Bundle::STATE_INSTALLED);

    bundleActivatorTransition4.Start();
    ASSERT_EQ(bundleActivatorTransition4.GetState(), Bundle::STATE_ACTIVE);

    bundleActivatorTransition4.Stop();
    ASSERT_EQ(bundleActivatorTransition4.GetState(), Bundle::STATE_UNINSTALLED);


    // Bundle TestBundleActivatorTransition5 calls Start on itself in its Bundle Activation Start function
    auto bundleActivatorTransition5 = InstallLib(context, "TestBundleActivatorTransition5");
    ASSERT_EQ(bundleActivatorTransition5.GetState(), Bundle::STATE_INSTALLED);

    bundleActivatorTransition5.Start();
    ASSERT_EQ(bundleActivatorTransition5.GetState(), Bundle::STATE_ACTIVE);

    // Bundle TestBundleActivatorTransition6 calls Stop on itself in its Bundle Activation Stop function
    auto bundleActivatorTransition6 = InstallLib(context, "TestBundleActivatorTransition6");
    ASSERT_EQ(bundleActivatorTransition6.GetState(), Bundle::STATE_INSTALLED);

    bundleActivatorTransition6.Start();
    ASSERT_EQ(bundleActivatorTransition6.GetState(), Bundle::STATE_ACTIVE);

    bundleActivatorTransition6.Stop();
    ASSERT_EQ(bundleActivatorTransition6.GetState(), Bundle::STATE_RESOLVED);
}

TEST_F(BundleLifecycleTest, TestStartStopDroppedTransitions)
{
    std::map<Bundle::State, int> observed;
    int iterations = 100;

    std::vector<Bundle::State> expectedStates = {
      Bundle::STATE_ACTIVE,
      Bundle::STATE_RESOLVED
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

        ASSERT_TRUE(state == Bundle::STATE_ACTIVE ||
                state == Bundle::STATE_RESOLVED)
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

    std::map<Bundle::State, int> observed;
    int iterations = 100;

    std::vector<Bundle::State> expectedStates = {
      Bundle::STATE_ACTIVE,
      Bundle::STATE_RESOLVED,
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

        ASSERT_TRUE(state == Bundle::STATE_ACTIVE ||
                state == Bundle::STATE_RESOLVED ||
                state == Bundle::STATE_UNINSTALLED)
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
  auto bundle = cppmicroservices::testing::InstallLib(context, "TestBundleMissingDestroyActivator");

  EXPECT_THROW(
      bundle.Start(),
      std::runtime_error);
}

TEST_F(BundleLifecycleTest, TestStartedBundleListenerThrowsSecurityException)
{
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

TEST_F(BundleLifecycleTest, TestStartFailedRaceWithStart)
{

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
