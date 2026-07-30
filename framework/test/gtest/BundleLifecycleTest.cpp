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

TEST_F(BundleLifecycleTest, TestState)
{
    auto bundleA = InstallLib(context, "TestBundleA");
    ASSERT_TRUE(bundleA);
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_INSTALLED);
    bundleA.Stop();
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_INSTALLED);
    bundleA.Start();
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_ACTIVE);
    bundleA.Uninstall();
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_UNINSTALLED);
}

TEST_F(BundleLifecycleTest, TestState2)
{
    auto bundleA = InstallLib(context, "TestBundleA");
    ASSERT_TRUE(bundleA);
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_INSTALLED);
    bundleA.Start();
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_ACTIVE);
    bundleA.Stop();
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_RESOLVED);
    bundleA.Stop();
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_RESOLVED);
    bundleA.Start();
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_ACTIVE);
    bundleA.Uninstall();
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_UNINSTALLED);
}

TEST_F(BundleLifecycleTest, TestLogs)
{
    auto logger = std::make_shared<MockLogger>();
    auto loggerReg = context.RegisterService<logservice::LogService>(logger);

    ON_CALL(*logger, Log(::testing::_, ::testing::_))
        .WillByDefault([](logservice::SeverityLevel, std::string const& msg) {
            std::cerr << msg << std::endl;
        });

    EXPECT_CALL(*logger, Log(::testing::_, ::testing::_)).Times(::testing::AnyNumber());

    auto bundleA = InstallLib(context, "TestBundleA");
    auto bundleB = InstallLib(context, "TestBundleB");
    ASSERT_TRUE(bundleA);
    ASSERT_TRUE(bundleB);

    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_INSTALLED);
    bundleA.Stop();
    bundleA.Start();
    bundleA.Stop();
    bundleA.Start();
    bundleA.Stop();
    bundleA.Start();
    bundleA.Stop();
    bundleA.Start();
    bundleA.Uninstall();
    ASSERT_EQ(bundleA.GetState(), Bundle::STATE_UNINSTALLED);

    ASSERT_EQ(bundleB.GetState(), Bundle::STATE_INSTALLED);
    bundleB.Stop();
    bundleB.Start();
    bundleB.Stop();
    bundleB.Start();
    bundleB.Stop();
    bundleB.Start();
    bundleB.Stop();
    bundleB.Start();
    bundleB.Uninstall();
    ASSERT_EQ(bundleB.GetState(), Bundle::STATE_UNINSTALLED);
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