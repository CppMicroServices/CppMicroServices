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

auto const DEFAULT_POLL_PERIOD = std::chrono::milliseconds(90000);

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
std::pair<bool, std::string> pollOnCondition(
  Func&& func,
  int compareTo,
  std::chrono::milliseconds const& timeout = DEFAULT_POLL_PERIOD)
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

  auto const startTime = std::chrono::steady_clock::now();
  auto result = pollingFunc();

  while (!result &&
         std::chrono::duration_cast<std::chrono::milliseconds>(
           std::chrono::steady_clock::now() - startTime) <= timeout) {
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

    auto const numBundles =
      installAndStartTestBundles(bc, "ManagedServiceAndFactoryBundle");

    auto sr = bc.GetServiceReference<
      cppmicroservices::service::cm::ConfigurationAdmin>();
    m_configAdmin = bc.GetService<cm::ConfigurationAdmin>(sr);

    ASSERT_EQ(numBundles, 1);
  }

  void TearDown() override
  {
    m_configAdmin.reset();

    // Shut down the framework, so it's restarted in the next testpoint.
    m_framework.Stop();
    m_framework.WaitForStop(std::chrono::milliseconds::zero());
  }

  cppmicroservices::Framework& GetFramework() { return m_framework; }

protected:
  std::shared_ptr<cm::ConfigurationAdmin> m_configAdmin;

private:
  cppmicroservices::Framework m_framework;
};

TEST_F(ConfigAdminTests, testInstallAndStart)
{
  auto f = GetFramework();
  auto ctx = f.GetBundleContext();

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

  auto const service = getManagedService(ctx);
  ASSERT_NE(service, nullptr);

  // We should get at least one Updated() call with the initial configuration from
  // the test bundle's manifest.json file (anInt=2). The asynchronous nature of
  // ConfigAdmin means that we may get more than one call to Updated(). This is also
  // why we check the counter is >= 1: we may get an initial Updated() call with empty
  // properties if the service is configured before the initial configuration has loaded.
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
  configuration->UpdateIfDifferent(props);

  // Poll on the service having been updated with the new configuration. The asynchronous
  // nature of configuration admin means that we may get an additional Updated() call, which
  // could have either anInt=2 or anInt=5 depending on relative thread timings.
  {
    bool result = false;
    std::string diagnostic;
    std::tie(result, diagnostic) = pollOnCondition<PollingCondition::GE>(
      [&service] { return service->getCounter(); }, expectedCount);
    EXPECT_TRUE(result) << diagnostic;
  }

  // Wait for long enough for any superfluous Updated() calls to wash out
  std::this_thread::sleep_for(DEFAULT_POLL_PERIOD);
  // At most, Updated() should have been called three times (one "excess"). If the excess
  // call were made with the updated properties, this would give us a maximum counter value
  // of 2 + 2*newIncrement. Update expectedCount to the current value of the counter.
  EXPECT_LE(service->getCounter(), expectedCount + newIncrement);
  expectedCount = service->getCounter();

  // UpdateIfDifferent with the same properties shouldn't call Updated()
  configuration->UpdateIfDifferent(props);
  std::this_thread::sleep_for(DEFAULT_POLL_PERIOD);
  EXPECT_EQ(service->getCounter(), expectedCount);

  // Update should call Updated() even if the properties are unchanged
  configuration->Update(props);
  expectedCount += newIncrement;
  {
    bool result = false;
    std::string diagnostic;
    std::tie(result, diagnostic) = pollOnCondition<PollingCondition::EQ>(
      [&service] { return service->getCounter(); }, expectedCount);
    EXPECT_TRUE(result) << diagnostic;
  }
}

TEST_F(ConfigAdminTests, testServiceRemoved)
{
  auto f = GetFramework();
  auto ctx = f.GetBundleContext();

  auto const service = getManagedService(ctx);
  ASSERT_NE(service, nullptr);

  // Sleep rather than poll to allow superfluous Updated() calls to flush through
  std::this_thread::sleep_for(DEFAULT_POLL_PERIOD);
  EXPECT_GE(service->getCounter(), 1);

  auto const initConfiguredCount = service->getCounter();
  int expectedCount = initConfiguredCount;

  auto configuration = m_configAdmin->GetConfiguration("cm.testservice");

  configuration->Remove();
  expectedCount -= 1;

  {
    bool result = false;
    std::string diagnostic;
    std::tie(result, diagnostic) = pollOnCondition<PollingCondition::EQ>(
      [&service] { return service->getCounter(); }, expectedCount);
    EXPECT_TRUE(result) << diagnostic;
  }

  // Should create a new configuration and call Updated()
  configuration = m_configAdmin->GetConfiguration("cm.testservice");
  EXPECT_TRUE(configuration->GetProperties().empty());
  expectedCount -= 1;

  {
    bool result = false;
    std::string diagnostic;
    std::tie(result, diagnostic) = pollOnCondition<PollingCondition::EQ>(
      [&service] { return service->getCounter(); }, expectedCount);
    EXPECT_TRUE(result) << diagnostic;
  }

  const int newIncrement{ 5 };
  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  props["anInt"] = newIncrement;
  expectedCount += newIncrement;

  // props differs from the latest configuration for the recreated configuration, which is empty.
  // The service should therefore get an Updated() notification.
  configuration->UpdateIfDifferent(props);
  {
    bool result = false;
    std::string diagnostic;
    std::tie(result, diagnostic) = pollOnCondition<PollingCondition::EQ>(
      [&service] { return service->getCounter(); }, expectedCount);
    EXPECT_TRUE(result) << diagnostic;
  }
}

/*
 * Managed service factory tests
 */
TEST_F(ConfigAdminTests, testServiceFactoryUpdated)
{
  auto f = GetFramework();
  auto ctx = f.GetBundleContext();

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
  configuration_config1->UpdateIfDifferent(props);
  {
    bool result = false;
    std::string diagnostic;
    std::tie(result, diagnostic) = pollOnCondition<PollingCondition::EQ>(
      [&serviceFactory] {
        return serviceFactory->getUpdatedCounter("cm.testfactory~config1");
      },
      expectedCount_config1);
    EXPECT_TRUE(result) << diagnostic;
  }
  EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config2"),
            expectedCount_config2);

  // UpdateIfDifferent with the same properties shouldn't call Updated()
  configuration_config1->UpdateIfDifferent(props);
  std::this_thread::sleep_for(DEFAULT_POLL_PERIOD);
  EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config1"),
            expectedCount_config1);
  EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config2"),
            expectedCount_config2);

  // Update should call Updated() even if the properties are unchanged
  configuration_config1->Update(props);
  expectedCount_config1 += newIncrement;
  {
    bool result = false;
    std::string diagnostic;
    std::tie(result, diagnostic) = pollOnCondition<PollingCondition::EQ>(
      [&serviceFactory] {
        return serviceFactory->getUpdatedCounter("cm.testfactory~config1");
      },
      expectedCount_config1);
    EXPECT_TRUE(result) << diagnostic;
  }
  EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config2"),
            expectedCount_config2);

  // Update config 2 now.
  auto configuration_config2 =
    m_configAdmin->GetFactoryConfiguration("cm.testfactory", "config2");
  configuration_config2->UpdateIfDifferent(props);
  expectedCount_config2 += newIncrement;

  {
    bool result = false;
    std::string diagnostic;
    std::tie(result, diagnostic) = pollOnCondition<PollingCondition::EQ>(
      [&serviceFactory] {
        return serviceFactory->getUpdatedCounter("cm.testfactory~config2");
      },
      expectedCount_config2);
    EXPECT_TRUE(result) << diagnostic;
  }
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

  auto const serviceFactory = getManagedServiceFactory(ctx);
  ASSERT_NE(serviceFactory, nullptr);

  auto configuration =
    m_configAdmin->CreateFactoryConfiguration("cm.testfactory");
  EXPECT_EQ(configuration->GetFactoryPid(), "cm.testfactory");

  auto const newConfigPid = configuration->GetPid();
  EXPECT_FALSE(newConfigPid.empty());
  EXPECT_NE(newConfigPid.find("cm.testfactory"), std::string::npos);

  EXPECT_TRUE(configuration->GetProperties().empty());

  {
    bool result = false;
    std::string diagnostic;
    std::tie(result, diagnostic) = pollOnCondition<PollingCondition::LT>(
      [&serviceFactory, &newConfigPid]() {
        return serviceFactory->getUpdatedCounter(newConfigPid);
      },
      0);
    EXPECT_TRUE(result) << diagnostic;
  }

  auto const service = serviceFactory->create(newConfigPid);
  ASSERT_NE(service, nullptr);
  EXPECT_LT(service->getValue(), 0);
}

TEST_F(ConfigAdminTests, testRemoveFactoryConfiguration)
{
  auto f = GetFramework();
  auto ctx = f.GetBundleContext();

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

  configuration_config1->Remove();

  {
    bool result = false;
    std::string diagnostic;
    std::tie(result, diagnostic) = pollOnCondition<PollingCondition::EQ>(
      [&serviceFactory] {
        return serviceFactory->getRemovedCounter("cm.testfactory~config1");
      },
      1);
    EXPECT_TRUE(result) << diagnostic;
  }
  EXPECT_EQ(serviceFactory->getRemovedCounter("cm.testfactory~config2"), 0);

  // Should create a new configuration
  configuration_config1 =
    m_configAdmin->GetFactoryConfiguration("cm.testfactory", "config1");
  --expectedCount_config1;

  EXPECT_TRUE(configuration_config1->GetProperties().empty());
  {
    bool result = false;
    std::string diagnostic;
    std::tie(result, diagnostic) = pollOnCondition<PollingCondition::EQ>(
      [&serviceFactory] {
        return serviceFactory->getUpdatedCounter("cm.testfactory~config1");
      },
      expectedCount_config1);
    EXPECT_TRUE(result) << diagnostic;
  }
  EXPECT_EQ(serviceFactory->getUpdatedCounter("cm.testfactory~config2"),
            expectedCount_config2);
}
