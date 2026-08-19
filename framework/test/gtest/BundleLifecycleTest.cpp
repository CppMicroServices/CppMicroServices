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

#include "cppmicroservices/util/FileSystem.h"
#include "cppmicroservices/util/String.h"

#include "FrameworkTestActivator.h"
#include "TestUtilBundleListener.h"
#include "TestUtilFrameworkListener.h"
#include "TestUtils.h"
#include "TestingConfig.h"

#include "gmock/gmock.h"
#include "cppmicroservices/logservice/LogService.hpp"

#include <chrono>
#include <future>
#include <thread>
#include <iostream>

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
    std::vector<BundleEvent> bundleEvents;
    context.AddBundleListener(&listener, &TestBundleListener::BundleChanged);

    auto bundleA = InstallLib(context, "TestBundleA");
    ASSERT_TRUE(bundleA);
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_INSTALLED);
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundleA));

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

    context.RemoveBundleListener(&listener, &TestBundleListener::BundleChanged);

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


TEST_F(BundleLifecycleTest, TestLogs)
{
    auto logger = std::make_shared<MockLogger>();
    auto loggerReg = context.RegisterService<logservice::LogService>(logger);

    // ON_CALL(*logger, Log(::testing::_, ::testing::_))
    //     .WillByDefault([](logservice::SeverityLevel, std::string const& msg) {
    //         std::cerr << msg << std::endl;
    //     });

    EXPECT_CALL(*logger, Log(::testing::_, ::testing::_)).Times(::testing::AnyNumber());

    auto bundleA = InstallLib(context, "TestBundleA");
    auto bundleB = InstallLib(context, "TestBundleB");
    ASSERT_TRUE(bundleA);
    ASSERT_TRUE(bundleB);
}

TEST_F(BundleLifecycleTest, TestConcurrency)
{

    auto logger = std::make_shared<MockLogger>();
    auto loggerReg = context.RegisterService<logservice::LogService>(logger);

    EXPECT_CALL(*logger, Log(::testing::_, ::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(*logger,
            Log(logservice::SeverityLevel::LOG_DEBUG,
                ::testing::HasSubstr("Dropped bundle lifecycle transition")))
    .Times(::testing::AtLeast(1));


    // ON_CALL(*logger, Log(::testing::_, ::testing::_))
    //     .WillByDefault([](logservice::SeverityLevel, std::string const& msg) {
    //         std::cerr << msg << std::endl;
    //     });


    auto bundleA = InstallLib(context, "TestBundleA");
    auto bundleB = InstallLib(context, "TestBundleB");
    ASSERT_TRUE(bundleA);
    ASSERT_TRUE(bundleB);

    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_INSTALLED);
    ASSERT_EQ(bundleB.GetState(), Bundle::STATE_INSTALLED);

    std::promise<void> go;
    std::shared_future<void> ready(go.get_future());
    constexpr int numCalls = 50;
    std::vector<std::promise<void>> readies(numCalls);
    std::vector<std::future<void>> bundleStateChanges(numCalls);

    for (int i = 0; i < numCalls; ++i)
    {
        auto bundle = (i % 4 < 2) ? bundleA : bundleB;
        bundleStateChanges[i] = std::async(
            std::launch::async,
            [bundle, ready, &readies, i]() mutable
            {
                readies[i].set_value();
                ready.wait();
                ((i % 2) ? bundle.Start() : bundle.Stop());
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

    bundleA.Uninstall();
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_UNINSTALLED);

    bundleB.Uninstall();
    ASSERT_EQ(bundleB.GetState(), Bundle::STATE_UNINSTALLED);
}