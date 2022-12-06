#include <gtest/gtest.h>

#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/cm/ConfigurationAdmin.hpp>
#include <cppmicroservices/util/FileSystem.h>

#include "TestInterfaces/Interfaces.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <utility>

#include "ConfigurationAdminTestingConfig.h"

namespace cm = cppmicroservices::service::cm;

namespace {

auto const DEFAULT_POLL_PERIOD = std::chrono::milliseconds(2000);

std::string PathToLib(const std::string& libName)
{
  return (cppmicroservices::testing::LIB_PATH +
          cppmicroservices::util::DIR_SEP + US_LIB_PREFIX + libName +
          US_LIB_POSTFIX + US_LIB_EXT);
}

std::string GetDSRuntimePluginFilePath()
{
  std::string libName{ "DeclarativeServices" };
#if defined(US_PLATFORM_WINDOWS)
  // This is a hack for the time being.
  // TODO: revisit changing the hard-coded "1" to the DS version dynamically
  libName += "1";
#endif
  return PathToLib(libName);
}

std::string GetConfigAdminRuntimePluginFilePath()
{
  std::string libName{ "ConfigurationAdmin" };
#if defined(US_PLATFORM_WINDOWS)
  libName += US_ConfigurationAdmin_VERSION_MAJOR;
#endif
  return PathToLib(libName);
}

void InstallAndStartDSAndConfigAdmin(::cppmicroservices::BundleContext& ctx)
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

  for (auto b : DSBundles) {
    b.Start();
  }

  for (auto b : CMBundles) {
    b.Start();
  }
}

size_t installAndStartTestBundles(cppmicroservices::BundleContext& ctx,
                                  const std::string& bundleName)
{
  std::string path = PathToLib(bundleName);
  auto bundles = ctx.InstallBundles(path);
  for (auto& b : bundles) {
    b.Start();
  }

  return bundles.size();
}

std::shared_ptr<::test::TestManagedServiceInterface> getManagedService(
  cppmicroservices::BundleContext& ctx)
{
  auto sr = ctx.GetServiceReference<::test::TestManagedServiceInterface>();
  return ctx.GetService<::test::TestManagedServiceInterface>(sr);
}

std::shared_ptr<::test::TestManagedServiceFactory> getManagedServiceFactory(
  cppmicroservices::BundleContext& ctx)
{
  auto sr = ctx.GetServiceReference<::test::TestManagedServiceFactory>();
  return ctx.GetService<::test::TestManagedServiceFactory>(sr);
}

enum class PollingCondition
{
  GT,
  GE,
  EQ,
  LE,
  LT
};
std::ostream& operator<<(std::ostream& out, PollingCondition condition)
{
  if (condition == PollingCondition::GT) {
    out << "greater than";
  } else if (condition == PollingCondition::GE) {
    out << "greater than or equal to";
  } else if (condition == PollingCondition::EQ) {
    out << "equal to";
  } else if (condition == PollingCondition::LE) {
    out << "less than or equal to";
  } else if (condition == PollingCondition::LT) {
    out << "less than";
  }
  return out;
}

template<PollingCondition condition, typename Func>
std::pair<bool, std::string> pollOnCondition(Func&& func, int compareTo)
{
  std::function<bool()> pollingFunc;
  if (condition == PollingCondition::GT) {
    pollingFunc = [&func, compareTo]() { return func() > compareTo; };
  } else if (condition == PollingCondition::GE) {
    pollingFunc = [&func, compareTo]() { return func() >= compareTo; };
  } else if (condition == PollingCondition::EQ) {
    pollingFunc = [&func, compareTo]() { return func() == compareTo; };
  } else if (condition == PollingCondition::LE) {
    pollingFunc = [&func, compareTo]() { return func() <= compareTo; };
  } else if (condition == PollingCondition::LT) {
    pollingFunc = [&func, compareTo]() { return func() < compareTo; };
  }

  auto result = pollingFunc();
  // HACK: Changing the CA threadpool to only use one thread causes the timing
  //       to change for how long it takes to get the Updated() notification.
  //       Rather than waiting for a certain amount of time, we wait indefinitely
  //       and run a hot loop until the condition is satisfied.
  while (!result) {
    result = pollingFunc();
  }

  std::string diagnostic("");
  if (!result) {
    std::stringstream ss;
    ss << "Expected polling function to be " << condition << " the value "
       << std::to_string(compareTo) << ", but was " << std::to_string(func());
    diagnostic = ss.str();
  }

  return { result, diagnostic };
}

} // namespace

class ConfigAdminTests : public ::testing::Test
{
public:
  ConfigAdminTests()
    : m_framework(cppmicroservices::FrameworkFactory().NewFramework())
  {}

  void SetUp() override
  {
    // Ensure the CppMicroServices framework is started. This should
    // install and start compendium services, including ConfigAdmin.
    m_framework.Start();
    auto bc = m_framework.GetBundleContext();

    InstallAndStartDSAndConfigAdmin(bc);

    // Instead of using BundleContext::GetBundles() and then filtering to
    // find the ConfigAdmin bundle, install the bundle again. This is harmless
    // since installing an already installed bundle is a no-op.
    auto installedBundles = m_framework.GetBundleContext().InstallBundles(
      GetConfigAdminRuntimePluginFilePath());
    ASSERT_EQ(installedBundles.size(), 1ul)
      << "Only one configadmin bundle should be installed";
    m_bundle = installedBundles.at(0);

    auto sr = bc.GetServiceReference<
      cppmicroservices::service::cm::ConfigurationAdmin>();
    m_configAdmin = bc.GetService<cm::ConfigurationAdmin>(sr);
  }

  void TearDown() override
  {
    m_configAdmin.reset();

    // Shut down the framework, so it's restarted in the next testpoint.
    m_framework.Stop();
    m_framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  cppmicroservices::Framework& GetFramework() { return m_framework; }
  cppmicroservices::Bundle& GetConfigAdminBundle() { return m_bundle; }

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

  auto const numBundles =
    installAndStartTestBundles(ctx, "ManagedServiceAndFactoryBundle");
  ASSERT_EQ(numBundles, 1ul);

  auto const service = getManagedService(ctx);
  EXPECT_NE(service, nullptr)
    << "Couldn't obtain test managed service implementation";

  auto const factory = getManagedServiceFactory(ctx);
  EXPECT_NE(factory, nullptr)
    << "Couldn't obtain test managed service factory implementation";
}

TEST_F(ConfigAdminTests, testServiceUpdated)
{
  auto f = GetFramework();
  auto ctx = f.GetBundleContext();

  auto const numBundles =
    installAndStartTestBundles(ctx, "ManagedServiceAndFactoryBundle");
  ASSERT_EQ(numBundles, 1ul);

  auto const service = getManagedService(ctx);
  ASSERT_NE(service, nullptr);

  // We should get one or two Updated() calls with the initial configuration from
  // the test bundle's manifest.json file (anInt=2). The asynchronous nature of
  // ConfigAdmin means that we may have to wait until the Updated call is received.
  {
    bool result = false;
    std::string diagnostic;
    std::tie(result, diagnostic) = pollOnCondition<PollingCondition::GE>(
      [&service] { return service->getCounter(); }, 1);
    EXPECT_TRUE(result) << diagnostic;
  }

  auto const initConfiguredCount = service->getCounter();
  int expectedCount = initConfiguredCount;

  auto configuration = m_configAdmin->GetConfiguration("cm.testservice");
  EXPECT_EQ(configuration->GetPid(), "cm.testservice");
  EXPECT_FALSE(configuration->GetProperties().empty());

  const int newIncrement{ 5 };
  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
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

TEST_F(ConfigAdminTests, testServiceRemoved)
{
  auto f = GetFramework();
  auto ctx = f.GetBundleContext();

  auto const numBundles =
    installAndStartTestBundles(ctx, "ManagedServiceAndFactoryBundle");
    ASSERT_EQ(numBundles, 1ul);

  auto const service = getManagedService(ctx);
  ASSERT_NE(service, nullptr);

  // Sleep rather than poll to allow superfluous Updated() calls to flush through
  std::this_thread::sleep_for(DEFAULT_POLL_PERIOD);
  EXPECT_GE(service->getCounter(), 1);

  auto const initConfiguredCount = service->getCounter();
  int expectedCount = initConfiguredCount;

  auto configuration = m_configAdmin->GetConfiguration("cm.testservice");

  EXPECT_EQ(service->getCounter(), expectedCount);

  // Remove sends an asynchronous notification to the ManagedService so we
  // have to wait until it's finished before checking the result.
  auto fut = configuration->Remove();
  fut.get();

  expectedCount -= 1;
  EXPECT_EQ(service->getCounter(), expectedCount);

  // Should create a new configuration and call Updated()
  // GetConfiguration doesn't send a notification to the ManagedService so
  // we don't have to wait for the result.
  configuration = m_configAdmin->GetConfiguration("cm.testservice");
  EXPECT_TRUE(configuration->GetProperties().empty());
  EXPECT_EQ(service->getCounter(), expectedCount);
 
  const int newIncrement{ 5 };
  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
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

  auto const numBundles =
    installAndStartTestBundles(ctx, "ManagedServiceAndFactoryBundle");
  ASSERT_EQ(numBundles, 1ul);

  auto const serviceFactory = getManagedServiceFactory(ctx);
  ASSERT_NE(serviceFactory, nullptr);

  // Sleep rather than poll to allow superfluous Updated() calls to flush through
  std::this_thread::sleep_for(DEFAULT_POLL_PERIOD);
  auto const initConfiguredCount_config1 =
    serviceFactory->getUpdatedCounter("cm.testfactory~config1");
  EXPECT_GE(initConfiguredCount_config1, 1);
  int expectedCount_config1 = initConfiguredCount_config1;

  auto const initConfiguredCount_config2 =
    serviceFactory->getUpdatedCounter("cm.testfactory~config2");
  EXPECT_GE(initConfiguredCount_config1, 1);
  int expectedCount_config2 = initConfiguredCount_config2;

  // Create services using the factory
  auto const svc1_before = serviceFactory->create("cm.testfactory~config1");
  EXPECT_EQ(svc1_before->getValue(), initConfiguredCount_config1);
  auto const svc2_before = serviceFactory->create("cm.testfactory~config2");
  EXPECT_EQ(svc2_before->getValue(), initConfiguredCount_config2);

  // Check the configuration config1 has the right properties
  auto configuration_config1 =
    m_configAdmin->GetFactoryConfiguration("cm.testfactory", "config1");
  EXPECT_EQ(configuration_config1->GetFactoryPid(), "cm.testfactory");
  EXPECT_EQ(configuration_config1->GetPid(), "cm.testfactory~config1");
  EXPECT_FALSE(configuration_config1->GetProperties().empty());

  // Update config1
  const int newIncrement{ 5 };
  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  props["anInt"] = newIncrement;

  expectedCount_config1 += newIncrement;

  // props differs from the initial configuration (anInt = 2) of the pid in the manifest file,
  // so the service should get an Updated() notification.
  auto fut = configuration_config1->UpdateIfDifferent(props);
  fut.second.get();
  EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config1"),
            expectedCount_config1);
  EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config2"),
            expectedCount_config2);

  // UpdateIfDifferent with the same properties shouldn't call Updated()
  fut = configuration_config1->UpdateIfDifferent(props);
  fut.second.get();
  EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config1"),
            expectedCount_config1);
  EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config2"),
            expectedCount_config2);

  // Update should call Updated() even if the properties are unchanged
  auto fut1 = configuration_config1->Update(props);
  fut1.get();
  expectedCount_config1 += newIncrement;
  EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config1"),
            expectedCount_config1);
  EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config2"),
            expectedCount_config2);

  // Update config 2 now.
  auto configuration_config2 =
    m_configAdmin->GetFactoryConfiguration("cm.testfactory", "config2");
  fut = configuration_config2->UpdateIfDifferent(props);
  fut.second.get();
  expectedCount_config2 += newIncrement;
  EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config2"),
            expectedCount_config2);
  EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config1"),
            expectedCount_config1);

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
 
  auto const numBundles =
    installAndStartTestBundles(ctx, "ManagedServiceAndFactoryBundle");
  ASSERT_EQ(numBundles, 1ul);

  auto const serviceFactory = getManagedServiceFactory(ctx);
  ASSERT_NE(serviceFactory, nullptr);

  auto configuration =
    m_configAdmin->CreateFactoryConfiguration("cm.testfactory");
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

  auto const numBundles =
    installAndStartTestBundles(ctx, "ManagedServiceAndFactoryBundle");
  ASSERT_EQ(numBundles, 1ul);

  auto const serviceFactory = getManagedServiceFactory(ctx);
  ASSERT_NE(serviceFactory, nullptr);

  // Sleep rather than poll to allow superfluous Updated() calls to flush through
  std::this_thread::sleep_for(DEFAULT_POLL_PERIOD);
  auto const initConfiguredCount_config1 =
    serviceFactory->getUpdatedCounter("cm.testfactory~config1");
  EXPECT_GE(initConfiguredCount_config1, 1);
  auto expectedCount_config1 = initConfiguredCount_config1;

  auto const initConfiguredCount_config2 =
    serviceFactory->getUpdatedCounter("cm.testfactory~config2");
  EXPECT_GE(initConfiguredCount_config1, 1);
  auto expectedCount_config2 = initConfiguredCount_config2;

  auto configuration_config1 =
    m_configAdmin->GetFactoryConfiguration("cm.testfactory", "config1");

  auto fut = configuration_config1->Remove();
  fut.get();

  EXPECT_NE(serviceFactory->getRemovedCounter("cm.testfactory~config1"), 0);
  EXPECT_EQ(serviceFactory->getRemovedCounter("cm.testfactory~config2"), 0);

  // Should create a new configuration
  configuration_config1 =
    m_configAdmin->GetFactoryConfiguration("cm.testfactory", "config1");

  EXPECT_TRUE(configuration_config1->GetProperties().empty());
  EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config1"),
            expectedCount_config1);
  EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config2"),
            expectedCount_config2);
}
// This test confirms that if an object exists in the configuration repository 
// but has not yet been Updated prior to the start of the ManagedServiceFactory
// then no Updated notification will be sent. 
TEST_F(ConfigAdminTests, testDuplicateUpdated)
{
  auto f = GetFramework();
  auto ctx = f.GetBundleContext();

  // Add cm.testfactory~0 configuration object to the configuration repository
  auto configuration = m_configAdmin->GetFactoryConfiguration("cm.testfactory","0");

  auto configurationMap =
      std::unordered_map<std::string, cppmicroservices::Any>{
        { "emgrid", std::to_string(0) }
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
