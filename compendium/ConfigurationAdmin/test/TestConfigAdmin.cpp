#include <gtest/gtest.h>

#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/ServiceProperties.h>
#include <cppmicroservices/cm/ConfigurationAdmin.hpp>
#include <cppmicroservices/util/FileSystem.h>

#include <cppmicroservices/cm/ManagedService.hpp>

#include "TestInterfaces/Interfaces.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <utility>

#include "ConfigurationAdminTestingConfig.h"

namespace cm = cppmicroservices::service::cm;

namespace
{

    auto const DEFAULT_POLL_PERIOD = std::chrono::milliseconds(2000);

    std::string
    PathToLib(std::string const& libName)
    {
        return (cppmicroservices::testing::LIB_PATH + cppmicroservices::util::DIR_SEP + US_LIB_PREFIX + libName
                + US_LIB_POSTFIX + US_LIB_EXT);
    }

    std::string
    GetDSRuntimePluginFilePath()
    {
        std::string libName { "DeclarativeServices" };
#if defined(US_PLATFORM_WINDOWS)
        // This is a hack for the time being.
        // TODO: revisit changing the hard-coded "1" to the DS version dynamically
        libName += "1";
#endif
        return PathToLib(libName);
    }

    std::string
    GetConfigAdminRuntimePluginFilePath()
    {
        std::string libName { "ConfigurationAdmin" };
#if defined(US_PLATFORM_WINDOWS)
        libName += US_ConfigurationAdmin_VERSION_MAJOR;
#endif
        return PathToLib(libName);
    }

    void
    InstallAndStartDSAndConfigAdmin(::cppmicroservices::BundleContext& ctx)
    {
        std::vector<cppmicroservices::Bundle> DSBundles;
        std::vector<cppmicroservices::Bundle> CMBundles;
        auto dsPluginPath = GetDSRuntimePluginFilePath();
        auto cmPluginPath = GetConfigAdminRuntimePluginFilePath();

#if defined(US_BUILD_SHARED_LIBS)
        DSBundles = ctx.InstallBundles(dsPluginPath);
        CMBundles = ctx.InstallBundles(cmPluginPath);
#else
        DSBundles = ctx.GetBundles();
        CMBundles = ctx.GetBundles();
#endif

        for (auto b : DSBundles)
        {
            b.Start();
        }

        for (auto b : CMBundles)
        {
            b.Start();
        }
    }

    std::vector<cppmicroservices::Bundle>
    InstallBundles(cppmicroservices::BundleContext& ctx, std::string const& bundleName)
    {
        std::string path = PathToLib(bundleName);
        return ctx.InstallBundles(path);
    }

    size_t
    installAndStartTestBundles(cppmicroservices::BundleContext& ctx, std::string const& bundleName)
    {
        auto bundles = InstallBundles(ctx, bundleName);
        for (auto& b : bundles)
        {
            b.Start();
        }

        return bundles.size();
    }

    std::shared_ptr<::test::TestManagedServiceInterface>
    getManagedService(cppmicroservices::BundleContext& ctx)
    {
        auto sr = ctx.GetServiceReference<::test::TestManagedServiceInterface>();
        return ctx.GetService<::test::TestManagedServiceInterface>(sr);
    }

    std::vector<std::shared_ptr<::test::TestManagedServiceInterface>>
    getManagedServices(cppmicroservices::BundleContext& ctx)
    {
        auto srs = ctx.GetServiceReferences<::test::TestManagedServiceInterface>();
        std::vector<std::shared_ptr<::test::TestManagedServiceInterface>> services;
        for (auto const& sr : srs)
        {
            services.push_back(ctx.GetService<::test::TestManagedServiceInterface>(sr));
        }

        return services;
    }

    std::shared_ptr<::test::TestManagedServiceFactory>
    getManagedServiceFactory(cppmicroservices::BundleContext& ctx)
    {
        auto sr = ctx.GetServiceReference<::test::TestManagedServiceFactory>();
        return ctx.GetService<::test::TestManagedServiceFactory>(sr);
    }

    std::vector<std::shared_ptr<::test::TestManagedServiceFactory>>
    getManagedServiceFactories(cppmicroservices::BundleContext& ctx)
    {
        auto srs = ctx.GetServiceReferences<::test::TestManagedServiceFactory>();
        std::vector<std::shared_ptr<::test::TestManagedServiceFactory>> factories;
        for (auto const& sr : srs)
        {
            factories.push_back(ctx.GetService<::test::TestManagedServiceFactory>(sr));
        }

        return factories;
    }

    enum class PollingCondition
    {
        GT,
        GE,
        EQ,
        LE,
        LT
    };
    std::ostream&
    operator<<(std::ostream& out, PollingCondition condition)
    {
        if (condition == PollingCondition::GT)
        {
            out << "greater than";
        }
        else if (condition == PollingCondition::GE)
        {
            out << "greater than or equal to";
        }
        else if (condition == PollingCondition::EQ)
        {
            out << "equal to";
        }
        else if (condition == PollingCondition::LE)
        {
            out << "less than or equal to";
        }
        else if (condition == PollingCondition::LT)
        {
            out << "less than";
        }
        return out;
    }

    template <PollingCondition condition, typename Func>
    std::pair<bool, std::string>
    pollOnCondition(Func&& func, int compareTo)
    {
        std::function<bool()> pollingFunc;
        if (condition == PollingCondition::GT)
        {
            pollingFunc = [&func, compareTo]() { return func() > compareTo; };
        }
        else if (condition == PollingCondition::GE)
        {
            pollingFunc = [&func, compareTo]() { return func() >= compareTo; };
        }
        else if (condition == PollingCondition::EQ)
        {
            pollingFunc = [&func, compareTo]() { return func() == compareTo; };
        }
        else if (condition == PollingCondition::LE)
        {
            pollingFunc = [&func, compareTo]() { return func() <= compareTo; };
        }
        else if (condition == PollingCondition::LT)
        {
            pollingFunc = [&func, compareTo]() { return func() < compareTo; };
        }

        auto result = pollingFunc();
        // HACK: Changing the CA threadpool to only use one thread causes the timing
        //       to change for how long it takes to get the Updated() notification.
        //       Rather than waiting for a certain amount of time, we wait indefinitely
        //       and run a hot loop until the condition is satisfied.
        while (!result)
        {
            result = pollingFunc();
        }

        std::string diagnostic("");
        if (!result)
        {
            std::stringstream ss;
            ss << "Expected polling function to be " << condition << " the value " << std::to_string(compareTo)
               << ", but was " << std::to_string(func());
            diagnostic = ss.str();
        }

        return { result, diagnostic };
    }

} // namespace

class ConfigAdminTests : public ::testing::Test
{
  public:
    ConfigAdminTests() : m_framework(cppmicroservices::FrameworkFactory().NewFramework()) {}

    void
    SetUp() override
    {
        // Ensure the CppMicroServices framework is started. This should
        // install and start compendium services, including ConfigAdmin.
        m_framework.Start();
        auto bc = m_framework.GetBundleContext();

        InstallAndStartDSAndConfigAdmin(bc);

        // Instead of using BundleContext::GetBundles() and then filtering to
        // find the ConfigAdmin bundle, install the bundle again. This is harmless
        // since installing an already installed bundle is a no-op.
        auto installedBundles = m_framework.GetBundleContext().InstallBundles(GetConfigAdminRuntimePluginFilePath());
        ASSERT_EQ(installedBundles.size(), 1ul) << "Only one configadmin bundle should be installed";
        m_bundle = installedBundles.at(0);

        auto sr = bc.GetServiceReference<cppmicroservices::service::cm::ConfigurationAdmin>();
        m_configAdmin = bc.GetService<cm::ConfigurationAdmin>(sr);
    }

    void
    TearDown() override
    {
        m_configAdmin.reset();

        // Shut down the framework, so it's restarted in the next testpoint.
        m_framework.Stop();
        m_framework.WaitForStop(std::chrono::milliseconds::zero());
    }

    cppmicroservices::Framework&
    GetFramework()
    {
        return m_framework;
    }
    cppmicroservices::Bundle&
    GetConfigAdminBundle()
    {
        return m_bundle;
    }

  protected:
    std::shared_ptr<cm::ConfigurationAdmin> m_configAdmin;

  private:
    cppmicroservices::Framework m_framework;
    cppmicroservices::Bundle m_bundle; ///< The ConfigAdmin bundle object
};

TEST_F(ConfigAdminTests, testProperties)
{
    // Test that the build system correctly generated the config admin bundle properties.
    auto b = GetConfigAdminBundle();
    ASSERT_EQ(b.GetSymbolicName(), US_ConfigurationAdmin_SYMBOLIC_NAME);
    ASSERT_EQ(b.GetVersion().ToString(), US_ConfigurationAdmin_VERSION_STR);
}

TEST_F(ConfigAdminTests, testInstallAndStart)
{
    auto f = GetFramework();
    auto ctx = f.GetBundleContext();

    auto const numBundles = installAndStartTestBundles(ctx, "ManagedServiceAndFactoryBundle");
    ASSERT_EQ(numBundles, 1ul);

    auto const service = getManagedService(ctx);
    EXPECT_NE(service, nullptr) << "Couldn't obtain test managed service implementation";

    auto const factory = getManagedServiceFactory(ctx);
    EXPECT_NE(factory, nullptr) << "Couldn't obtain test managed service factory implementation";
}

TEST_F(ConfigAdminTests, testServiceUpdated)
{
    auto f = GetFramework();
    auto ctx = f.GetBundleContext();

    auto const numBundles = installAndStartTestBundles(ctx, "ManagedServiceAndFactoryBundle");
    ASSERT_EQ(numBundles, 1ul);

    auto const service = getManagedService(ctx);
    ASSERT_NE(service, nullptr);

    // We should get one or two Updated() calls with the initial configuration from
    // the test bundle's manifest.json file (anInt=2). The asynchronous nature of
    // ConfigAdmin means that we may have to wait until the Updated call is received.
    {
        bool result = false;
        std::string diagnostic;
        std::tie(result, diagnostic)
            = pollOnCondition<PollingCondition::GE>([&service] { return service->getCounter(); }, 1);
        EXPECT_TRUE(result) << diagnostic;
    }

    auto const initConfiguredCount = service->getCounter();
    int expectedCount = initConfiguredCount;

    auto configuration = m_configAdmin->GetConfiguration("cm.testservice");
    EXPECT_EQ(configuration->GetPid(), "cm.testservice");
    EXPECT_FALSE(configuration->GetProperties().empty());

    int const newIncrement { 5 };
    cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["anInt"] = newIncrement;
    expectedCount += newIncrement;

    // props differs from the initial configuration (anInt = 2) of the pid in the manifest file,
    // so the service should get an Updated() notification.
    auto result = configuration->UpdateIfDifferent(props);
    result.second.get();
    EXPECT_EQ(service->getCounter(), expectedCount);

    // UpdateIfDifferent with the same properties shouldn't call Updated()
    result = configuration->UpdateIfDifferent(props);
    result.second.get();
    EXPECT_EQ(service->getCounter(), expectedCount);

    // Update should call Updated() even if the properties are unchanged
    auto fut = configuration->Update(props);
    fut.get();
    expectedCount += newIncrement;
    EXPECT_EQ(service->getCounter(), expectedCount);
}

TEST_F(ConfigAdminTests, testConfigurationRemoved)
{
    auto f = GetFramework();
    auto ctx = f.GetBundleContext();

    auto const numBundles = installAndStartTestBundles(ctx, "ManagedServiceAndFactoryBundle");
    ASSERT_EQ(numBundles, 1ul);

    auto const service = getManagedService(ctx);
    ASSERT_NE(service, nullptr);

    // Sleep rather than poll to allow superfluous Updated() calls to flush through
    std::this_thread::sleep_for(DEFAULT_POLL_PERIOD);
    EXPECT_GE(service->getCounter(), 1);

    auto expectedCount = service->getCounter();
    auto configuration = m_configAdmin->GetConfiguration("cm.testservice");

    EXPECT_EQ(service->getCounter(), expectedCount);

    // Remove sends an asynchronous notification to the ManagedService so we
    // have to wait until it's finished before checking the result.
    configuration->Remove().get();

    expectedCount -= 1;
    EXPECT_EQ(service->getCounter(), expectedCount);

    // Should create a new configuration and call Updated()
    // GetConfiguration doesn't send a notification to the ManagedService so
    // we don't have to wait for the result.
    configuration = m_configAdmin->GetConfiguration("cm.testservice");
    EXPECT_TRUE(configuration->GetProperties().empty());
    EXPECT_EQ(service->getCounter(), expectedCount);

    int const newIncrement { 5 };
    cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["anInt"] = newIncrement;
    expectedCount += newIncrement;

    // props differs from the latest configuration for the recreated configuration, which is empty.
    // The service should therefore get an Updated() notification.
    auto result = configuration->UpdateIfDifferent(props);
    result.second.get();
    EXPECT_EQ(service->getCounter(), expectedCount);
}

/*
 * Managed service factory tests
 */
TEST_F(ConfigAdminTests, testServiceFactoryUpdated)
{
    auto f = GetFramework();
    auto ctx = f.GetBundleContext();

    auto const numBundles = installAndStartTestBundles(ctx, "ManagedServiceAndFactoryBundle");
    ASSERT_EQ(numBundles, 1ul);

    auto const serviceFactory = getManagedServiceFactory(ctx);
    ASSERT_NE(serviceFactory, nullptr);

    // Sleep rather than poll to allow superfluous Updated() calls to flush through
    std::this_thread::sleep_for(DEFAULT_POLL_PERIOD);
    auto const initConfiguredCount_config1 = serviceFactory->getUpdatedCounter("cm.testfactory~config1");
    EXPECT_GE(initConfiguredCount_config1, 1);
    int expectedCount_config1 = initConfiguredCount_config1;

    auto const initConfiguredCount_config2 = serviceFactory->getUpdatedCounter("cm.testfactory~config2");
    EXPECT_GE(initConfiguredCount_config1, 1);
    int expectedCount_config2 = initConfiguredCount_config2;

    // Create services using the factory
    auto const svc1_before = serviceFactory->create("cm.testfactory~config1");
    EXPECT_EQ(svc1_before->getValue(), initConfiguredCount_config1);
    auto const svc2_before = serviceFactory->create("cm.testfactory~config2");
    EXPECT_EQ(svc2_before->getValue(), initConfiguredCount_config2);

    // Check the configuration config1 has the right properties
    auto configuration_config1 = m_configAdmin->GetFactoryConfiguration("cm.testfactory", "config1");
    EXPECT_EQ(configuration_config1->GetFactoryPid(), "cm.testfactory");
    EXPECT_EQ(configuration_config1->GetPid(), "cm.testfactory~config1");
    EXPECT_FALSE(configuration_config1->GetProperties().empty());

    // Update config1
    int const newIncrement { 5 };
    cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["anInt"] = newIncrement;

    expectedCount_config1 += newIncrement;

    // props differs from the initial configuration (anInt = 2) of the pid in the manifest file,
    // so the service should get an Updated() notification.
    auto fut = configuration_config1->UpdateIfDifferent(props);
    fut.second.get();
    EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config1"), expectedCount_config1);
    EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config2"), expectedCount_config2);

    // UpdateIfDifferent with the same properties shouldn't call Updated()
    fut = configuration_config1->UpdateIfDifferent(props);
    fut.second.get();
    EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config1"), expectedCount_config1);
    EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config2"), expectedCount_config2);

    // Update should call Updated() even if the properties are unchanged
    auto fut1 = configuration_config1->Update(props);
    fut1.get();
    expectedCount_config1 += newIncrement;
    EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config1"), expectedCount_config1);
    EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config2"), expectedCount_config2);

    // Update config 2 now.
    auto configuration_config2 = m_configAdmin->GetFactoryConfiguration("cm.testfactory", "config2");
    fut = configuration_config2->UpdateIfDifferent(props);
    fut.second.get();
    expectedCount_config2 += newIncrement;
    EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config2"), expectedCount_config2);
    EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config1"), expectedCount_config1);

    // Get new services from the factory. Check that subsequent updates to the
    // factory configuration doesn't affect already-created services.
    auto const svc1_after = serviceFactory->create("cm.testfactory~config1");
    EXPECT_EQ(svc1_after->getValue(), expectedCount_config1);
    auto const svc2_after = serviceFactory->create("cm.testfactory~config2");
    EXPECT_EQ(svc2_after->getValue(), expectedCount_config2);

    EXPECT_EQ(svc1_before->getValue(), initConfiguredCount_config1);
    EXPECT_EQ(svc2_before->getValue(), initConfiguredCount_config2);
}

TEST_F(ConfigAdminTests, testCreateFactoryConfiguration)
{
    auto f = GetFramework();
    auto ctx = f.GetBundleContext();

    auto const numBundles = installAndStartTestBundles(ctx, "ManagedServiceAndFactoryBundle");
    ASSERT_EQ(numBundles, 1ul);

    auto const serviceFactory = getManagedServiceFactory(ctx);
    ASSERT_NE(serviceFactory, nullptr);

    auto configuration = m_configAdmin->CreateFactoryConfiguration("cm.testfactory");
    EXPECT_EQ(configuration->GetFactoryPid(), "cm.testfactory");

    auto const newConfigPid = configuration->GetPid();
    EXPECT_FALSE(newConfigPid.empty());
    EXPECT_NE(newConfigPid.find("cm.testfactory"), std::string::npos);

    EXPECT_TRUE(configuration->GetProperties().empty());
    EXPECT_EQ(serviceFactory->getUpdatedCounter(newConfigPid), 0);

    auto const service = serviceFactory->create(newConfigPid);
    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->getValue(), 0);
}

TEST_F(ConfigAdminTests, testRemoveFactoryConfiguration)
{
    auto f = GetFramework();
    auto ctx = f.GetBundleContext();

    auto const numBundles = installAndStartTestBundles(ctx, "ManagedServiceAndFactoryBundle");
    ASSERT_EQ(numBundles, 1ul);

    auto const serviceFactory = getManagedServiceFactory(ctx);
    ASSERT_NE(serviceFactory, nullptr);

    // Sleep rather than poll to allow superfluous Updated() calls to flush through
    std::this_thread::sleep_for(DEFAULT_POLL_PERIOD);
    auto const initConfiguredCount_config1 = serviceFactory->getUpdatedCounter("cm.testfactory~config1");
    EXPECT_GE(initConfiguredCount_config1, 1);
    auto expectedCount_config1 = initConfiguredCount_config1;

    auto const initConfiguredCount_config2 = serviceFactory->getUpdatedCounter("cm.testfactory~config2");
    EXPECT_GE(initConfiguredCount_config1, 1);
    auto expectedCount_config2 = initConfiguredCount_config2;

    auto configuration_config1 = m_configAdmin->GetFactoryConfiguration("cm.testfactory", "config1");

    configuration_config1->Remove().get();

    EXPECT_EQ(serviceFactory->getRemovedCounter("cm.testfactory~config1"), 1);
    EXPECT_EQ(serviceFactory->getRemovedCounter("cm.testfactory~config2"), 0);

    // Should create a new configuration
    configuration_config1 = m_configAdmin->GetFactoryConfiguration("cm.testfactory", "config1");

    EXPECT_TRUE(configuration_config1->GetProperties().empty());
    EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config1"), expectedCount_config1);
    EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config2"), expectedCount_config2);

    cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["anInt"] = 5;
    configuration_config1->UpdateIfDifferent(props).second.get();
    expectedCount_config1 = initConfiguredCount_config1;
    EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config1"), 7);
}
// This test confirms that if an object exists in the configuration repository
// but has not yet been Updated prior to the start of the ManagedServiceFactory
// then no Updated notification will be sent.
TEST_F(ConfigAdminTests, testDuplicateUpdated)
{
    auto f = GetFramework();
    auto ctx = f.GetBundleContext();

    // Add cm.testfactory~0 configuration object to the configuration repository
    auto configuration = m_configAdmin->GetFactoryConfiguration("cm.testfactory", "0");

    auto configurationMap = std::unordered_map<std::string, cppmicroservices::Any> {
        {"emgrid", std::to_string(0)}
    };

    // Start the ManagedServiceFactory for cm.testfactory. Since the
    // cm.testfactory~0 instance has not yet been updated, no Update
    // notification will be sent to the ManagedServiceFactory.
    installAndStartTestBundles(ctx, "TestBundleManagedServiceFactory");

    // Update the cm.testfactory~0 configuration object. An Update
    // notification will be sent to the ManagedServiceFactory.
    auto result = configuration->UpdateIfDifferent(configurationMap);
    result.second.get();

    auto const serviceFactory = getManagedServiceFactory(ctx);
    ASSERT_NE(serviceFactory, nullptr);

    EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~0"), 1);
}

/// <summary>
/// Used by ConfigAdminTests.testConcurrentDuplicateManagedServiceUpdated
/// </summary>
namespace cppmicroservices
{
    namespace test
    {
        class TestManagedServiceInterface : public cppmicroservices::service::cm::ManagedService
        {
          public:
            virtual ~TestManagedServiceInterface() noexcept = default;

            void Updated(cppmicroservices::AnyMap const&) override = 0;
            virtual unsigned long getUpdatedMethodCallCount() noexcept = 0;
        };

        class TestManagedService : public TestManagedServiceInterface
        {
          public:
            TestManagedService() : updatedCount_ { 0 } {}
            virtual ~TestManagedService() noexcept = default;

            void
            Updated(cppmicroservices::AnyMap const& props) override
            {
                std::unique_lock<std::mutex> lock(updatedCountMutex_);
                // empty properties can be sent when stopping the configadmin
                // service. For the purpose of this test we only want to
                // update the count when non-empty properties have been sent.
                if (!props.empty())
                {
                    updatedCount_++;
                }
            }

            unsigned long
            getUpdatedMethodCallCount() noexcept override
            {
                std::unique_lock<std::mutex> lock(updatedCountMutex_);
                return updatedCount_;
            }

          private:
            unsigned long updatedCount_;
            std::mutex updatedCountMutex_;
        };
    } // namespace test
} // namespace cppmicroservices

// This test simulates sending a config update when two threads are
// racing to register a ManagedService and update the configuration
// object.
// This test is meant to be run in a loop to detect race conditions.
TEST_F(ConfigAdminTests, testConcurrentDuplicateManagedServiceUpdated)
{
    auto f = GetFramework();
    auto ctx = f.GetBundleContext();

    std::promise<void> go;
    std::shared_future<void> ready(go.get_future());
    std::vector<std::promise<void>> readies(2);

    auto registerManagedService
        = std::async(std::launch::async,
                     [&ready, &readies, &ctx]()
                     {
                         readies[0].set_value();
                         ready.wait();

                         cppmicroservices::ServiceProperties serviceProperties;
                         serviceProperties["service.pid"] = cppmicroservices::Any(std::string("cm.testservice"));
                         (void)ctx.RegisterService<cppmicroservices::test::TestManagedServiceInterface,
                                                   cppmicroservices::service::cm::ManagedService>(
                             std::make_shared<cppmicroservices::test::TestManagedService>(),
                             serviceProperties);
                     });

    auto updateConfigFuture = std::async(std::launch::async,
                                         [this, &ready, &readies]()
                                         {
                                             readies[1].set_value();
                                             ready.wait();

                                             // Add cm.testservice configuration object to the configuration repository
                                             auto configuration = m_configAdmin->GetConfiguration("cm.testservice");

                                             auto configurationMap
                                                 = std::unordered_map<std::string, cppmicroservices::Any> {
                                                       {"emgrid", std::to_string(0)}
                                             };
                                             // Update the cm.testservice configuration object. An Update
                                             // notification will be sent to the ManagedServiceFactory.
                                             auto result = configuration->UpdateIfDifferent(configurationMap);
                                             result.second.get();
                                         });

    readies[0].get_future().wait();
    readies[1].get_future().wait();
    go.set_value();

    ASSERT_NO_THROW(registerManagedService.get());
    ASSERT_NO_THROW(updateConfigFuture.get());

    auto sr = ctx.GetServiceReference<cppmicroservices::test::TestManagedServiceInterface>();
    auto managedService = ctx.GetService<cppmicroservices::test::TestManagedServiceInterface>(sr);
    ASSERT_NE(managedService, nullptr);

    // wait for config admin to finish processing all config events
    // by stopping the configadmin bundle. This is necessary to guarantee
    // that when we check for the # of Updated method calls, a config
    // admin thread isn't still processing one.
    auto configAdminBundle = GetConfigAdminBundle();
    configAdminBundle.Stop();
    m_configAdmin.reset();
    EXPECT_EQ(managedService->getUpdatedMethodCallCount(), 1);
}

// This test simulates sending a config update when two threads are
// racing to register a ManagedServiceFactory and update the configuration
// object.
// Thgis test is meant to be run in a loop to detect race conditions.
TEST_F(ConfigAdminTests, testConcurrentDuplicateManagedServiceFactoryUpdated)
{
    auto f = GetFramework();
    auto ctx = f.GetBundleContext();

    std::promise<void> go;
    std::shared_future<void> ready(go.get_future());
    std::vector<std::promise<void>> readies(2);

    auto installAndStartBundleFuture
        = std::async(std::launch::async,
                     [&ready, &readies, &ctx]()
                     {
                         readies[0].set_value();
                         ready.wait();
                         // Start the ManagedServiceFactory for cm.testfactory. Since the
                         // cm.testfactory~0 instance has not yet been updated, no Update
                         // notification will be sent to the ManagedServiceFactory.
                         installAndStartTestBundles(ctx, "TestBundleManagedServiceFactory");
                     });

    auto updateConfigFuture
        = std::async(std::launch::async,
                     [this, &ready, &readies]()
                     {
                         readies[1].set_value();
                         ready.wait();

                         // Add cm.testfactory~0 configuration object to the configuration repository
                         auto configuration = m_configAdmin->GetFactoryConfiguration("cm.testfactory", "0");

                         auto configurationMap = std::unordered_map<std::string, cppmicroservices::Any> {
                             {"emgrid", std::to_string(0)}
                         };
                         // Update the cm.testfactory~0 configuration object. An Update
                         // notification will be sent to the ManagedServiceFactory.
                         auto result = configuration->UpdateIfDifferent(configurationMap);
                         result.second.get();
                     });

    readies[0].get_future().wait();
    readies[1].get_future().wait();
    go.set_value();

    ASSERT_NO_THROW(installAndStartBundleFuture.get());
    ASSERT_NO_THROW(updateConfigFuture.get());

    auto const serviceFactory = getManagedServiceFactory(ctx);
    ASSERT_NE(serviceFactory, nullptr);

    // wait for config admin to finish processing all config events
    // by stopping the configadmin bundle. This is necessary to guarantee
    // that when we check for the # of Updated method calls, a config
    // admin thread isn't still processing one.
    auto configAdminBundle = GetConfigAdminBundle();
    configAdminBundle.Stop();
    m_configAdmin.reset();
    EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~0"), 1);
}

/// Tests a DS bundle with a ManagedService service which starts another
/// DS bundle with a ManagedService service within its Activate method. This test
/// is meant to simulate nested calls to Configuration Admin's service trackers
/// and test for deadlocks.
TEST_F(ConfigAdminTests, testNestedBundleInstallAndStart)
{
    auto f = GetFramework();
    auto ctx = f.GetBundleContext();

    // install, but don't start, the managed service test bundle. Start will
    // happen within the Activate of TestBundleNestedBundleStartManagedService
    // It is done this way so that the test bundles don't have to have knowledge
    // about the paths to test bundles.
    auto bundles = ctx.InstallBundles(PathToLib("ManagedServiceAndFactoryBundle"));
    ASSERT_EQ(1ul, bundles.size());

    // This bundle's Activate method has a call to start TestBundleManagedService, which will trigger
    // a call to Config Admin's service tracker.
    auto const numBundles = installAndStartTestBundles(ctx, "TestBundleNestedBundleStartManagedService");
    ASSERT_EQ(numBundles, 1ul);
}

TEST_F(ConfigAdminTests, testMultipleManagedServicesInBundleGetUpdate)
{
    auto f = GetFramework();
    auto ctx = f.GetBundleContext();

    auto const numBundles = installAndStartTestBundles(ctx, "TestBundleMultipleManagedService");
    ASSERT_EQ(numBundles, 1);

    auto services = getManagedServices(ctx);
    ASSERT_EQ(services.size(), 2ul);
    std::for_each(std::begin(services), std::end(services), [](auto& service) { ASSERT_NE(service, nullptr); });

    auto sharedConfiguration = m_configAdmin->GetConfiguration("cm.testservice");
    ASSERT_EQ(sharedConfiguration->GetPid(), "cm.testservice");
    ASSERT_TRUE(sharedConfiguration->GetProperties().empty());

    std::for_each(std::begin(services), std::end(services), [](auto& service) { ASSERT_EQ(service->getCounter(), 0); });

    cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["myProp"] = 0;

    auto fut = sharedConfiguration->Update(props);
    fut.get();

    std::for_each(std::begin(services), std::end(services), [](auto& service) { ASSERT_EQ(service->getCounter(), 1); });
}

TEST_F(ConfigAdminTests, testMultipleManagedFactoriesInBundleGetUpdate)
{
    auto f = GetFramework();
    auto ctx = f.GetBundleContext();

    auto const numBundles = installAndStartTestBundles(ctx, "TestBundleMultipleManagedServiceFactory");
    ASSERT_EQ(numBundles, 1);

    auto serviceFactories = getManagedServiceFactories(ctx);
    ASSERT_EQ(serviceFactories.size(), 2ul);
    std::for_each(std::begin(serviceFactories),
                  std::end(serviceFactories),
                  [](auto& factory) { ASSERT_NE(factory, nullptr); });

    auto sharedConfiguration = m_configAdmin->GetFactoryConfiguration("cm.testfactory", "ver1");
    ASSERT_EQ(sharedConfiguration->GetFactoryPid(), "cm.testfactory");
    ASSERT_TRUE(sharedConfiguration->GetProperties().empty());

    std::for_each(std::begin(serviceFactories),
                  std::end(serviceFactories),
                  [](auto& factory) { ASSERT_EQ(factory->getUpdatedCounter("cm.testfactory~ver1"), 0); });

    cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["myProp"] = 0;

    auto fut = sharedConfiguration->Update(props);
    fut.get();

    std::for_each(std::begin(serviceFactories),
                  std::end(serviceFactories),
                  [](auto& factory) { ASSERT_EQ(factory->getUpdatedCounter("cm.testfactory~ver1"), 1); });
}
// <summary>
/// Used by ConfigAdminTests.testManagedServiceRemoveConfigurationsDeadlock
/// </summary>
namespace cppmicroservices
{
    namespace test
    {
        class TestManagedServiceInterface2 : public cppmicroservices::service::cm::ManagedService
        {
          public:
            virtual ~TestManagedServiceInterface2() noexcept = default;
            void Updated(cppmicroservices::AnyMap const&) override = 0;
        };

        class TestManagedServiceImpl : public TestManagedServiceInterface2
        {
          public:
            TestManagedServiceImpl(cppmicroservices::Framework& framework) : m_framework(framework) {}
            virtual ~TestManagedServiceImpl() noexcept = default;

            void
            Updated(cppmicroservices::AnyMap const& props) override
            {
                // The Updated method is called for both Updated and Removed operations.
                // For the Removed operations, the properties are empty.
                if (props.empty())
                {
                    auto ctx = m_framework.GetBundleContext();
                    auto bundles = ctx.GetBundles(PathToLib("TestBundleManagedServiceDeadlock"));
                    ASSERT_EQ(bundles.size(), 1ul);
                    for (auto& b : bundles)
                    {
                        ASSERT_EQ(b.GetSymbolicName(), "TestBundleManagedServiceDeadlock");
                        b.Stop();
                    }
                }
            }

          private:
            cppmicroservices::Framework m_framework;
        };
    } // namespace test
} // namespace cppmicroservices
/*
 * testManagedServiceRemoveConfigurationsDeadlock
 *
 * This test was added in response to a deadlock bug.
 * The Use Case is as follows:
 *     A configuration object is defined in the manifest.json file. (In
 *        this case it is defined in TestBundleManagedServiceDeadlock)
 *     The User's main thread stopped the bundle which causes
 *         the ConfigurationAdminImpl RemoveConfigurations method
 *         to remove the configuration object from the ConfigurationAdmin
 *         repository and to send a Removed notification to the service
 *         instance. RemoveConfigurations would then execute a WaitForAllAsync
 *         to wait for all asynchronous threads to complete including the
 *         asynchronous thread that was launched as part of the Removed
 *         notification.
 *      The user's Updated method also tried to stop the bundle. This means
 *          that it had to wait for the RemoveConfigurations method to complete.
 *      Hence the deadlock.
 *
 * The solution is to remove the WaitForAllAsync from RemoveConfigurations.
 * This test will deadlock if the WaitForAllAsync function is not removed.
 *
 */
TEST_F(ConfigAdminTests, testManagedServiceRemoveConfigurationsDeadlock)
{
    auto f = GetFramework();
    auto ctx = f.GetBundleContext();

    // Install and start the bundle containing the cm.testdeadlock configuration object.
    auto bundles = InstallBundles(ctx, "TestBundleManagedServiceDeadlock");
    ASSERT_FALSE(bundles.empty());
    for (auto& b : bundles)
    {
        b.Start();
    }

    // Register the service that implmenets the TestManagedServiceInterface2
    // and ManagedService interface. The implementation class is TestManagedServiceImpl
    // This service receives notifications when the cm.testdeadlock configuration object
    // is updated or removed.
    cppmicroservices::ServiceProperties serviceProperties;
    serviceProperties["service.pid"] = cppmicroservices::Any(std::string("cm.testdeadlock"));
    (void)ctx.RegisterService<cppmicroservices::test::TestManagedServiceInterface2,
                              cppmicroservices::service::cm::ManagedService>(
        std::make_shared<cppmicroservices::test::TestManagedServiceImpl>(f),
        serviceProperties);

    // Stop the bundle containing the cm.testdeadlock configuration object. This causes
    // ConfigurationAdminImpl::RemoveConfigurations to execute. It sends an Updated notification
    // to the TestManagedServiceImpl service. With the WaitForAllAsync method present in
    // in RemoveConfigurations, the bundle Stop command won't return until the Updated
    // notification is complete. Unfortunately, the Updated method also tries to stop the
    // bundle and a deadlock results.
    for (auto& b : bundles)
    {
        b.Stop();
    }
}