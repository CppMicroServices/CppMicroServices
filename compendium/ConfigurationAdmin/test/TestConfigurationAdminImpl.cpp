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

#include <sstream>

#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/cm/ConfigurationException.hpp"

#include "../src/ConfigurationAdminImpl.hpp"
#include "Mocks.hpp"

namespace cppmicroservices {
  namespace cmimpl {
    MATCHER_P(AnyMapEquals, value, "")
    {
      std::ostringstream argString;
      std::ostringstream valueString;
      cppmicroservices::any_value_to_string(argString, arg);
      cppmicroservices::any_value_to_string(valueString, value);
      return (argString.str() == valueString.str());
    }

    // The fixture for testing class ConfigurationAdminImpl.
    class TestConfigurationAdminImpl : public ::testing::Test {
    protected:
      TestConfigurationAdminImpl() : framework(cppmicroservices::FrameworkFactory().NewFramework()) {
      }

      ~TestConfigurationAdminImpl() override = default;

      void SetUp() override {
        framework.Start();
      }

      void TearDown() override {
        framework.Stop();
        framework.WaitForStop(std::chrono::milliseconds::zero());
      }

      cppmicroservices::Framework& GetFramework() { return framework; }
    private:
      cppmicroservices::Framework framework;
    };

    TEST_F(TestConfigurationAdminImpl, VerifyGetConfiguration)
    {
      auto bundleContext = GetFramework().GetBundleContext();
      auto fakeLogger = std::make_shared<FakeLogger>();
      ConfigurationAdminImpl configAdmin(bundleContext, fakeLogger);

      {
        const auto conf = configAdmin.GetConfiguration("test.pid");
        ASSERT_TRUE(conf);
        EXPECT_EQ(conf->GetPid(), "test.pid");
        EXPECT_EQ(conf->GetFactoryPid(), "");
        auto props = conf->GetProperties();
        EXPECT_THAT(props, testing::SizeIs(0));
        props["foo"] = std::string{"bar"};
        EXPECT_NO_THROW(conf->Update(props));

        // Multiple requests should return the same object
        const auto conf2 = configAdmin.GetConfiguration("test.pid");
        EXPECT_EQ(conf, conf2);
        EXPECT_THAT(conf2->GetProperties(), testing::SizeIs(1));
      }

      // Same test with a Factory PID
      {
        const auto factoryConf = configAdmin.GetConfiguration("factory~instance1");
        ASSERT_TRUE(factoryConf);
        EXPECT_EQ(factoryConf->GetPid(), "factory~instance1");
        EXPECT_EQ(factoryConf->GetFactoryPid(), "factory");
        EXPECT_THAT(factoryConf->GetProperties(), testing::SizeIs(0));

        const auto factoryConf2 = configAdmin.GetConfiguration("factory~instance1");
        EXPECT_EQ(factoryConf, factoryConf2);
      }

      // Test GetConfiguration after removal
      const auto conf = configAdmin.GetConfiguration("test.pid");
      ASSERT_TRUE(conf);
      EXPECT_NO_THROW(conf->Remove());

      const auto conf2 = configAdmin.GetConfiguration("test.pid");
      ASSERT_TRUE(conf2);
      EXPECT_EQ(conf2->GetPid(), "test.pid");
      EXPECT_NE(conf, conf2);
    }

    TEST_F(TestConfigurationAdminImpl, VerifyCreateFactoryConfiguration)
    {
      auto bundleContext = GetFramework().GetBundleContext();
      auto fakeLogger = std::make_shared<FakeLogger>();
      ConfigurationAdminImpl configAdmin(bundleContext, fakeLogger);

      const auto conf = configAdmin.CreateFactoryConfiguration("factory");
      ASSERT_TRUE(conf);
      EXPECT_THAT(conf->GetPid(), testing::HasSubstr("factory~"));
      EXPECT_EQ(conf->GetFactoryPid(), "factory");
      EXPECT_THAT(conf->GetProperties(), testing::SizeIs(0));

      const auto conf2 = configAdmin.CreateFactoryConfiguration("factory");
      ASSERT_TRUE(conf2);
      EXPECT_THAT(conf2->GetPid(), testing::HasSubstr("factory~"));
      EXPECT_EQ(conf2->GetFactoryPid(), "factory");
      EXPECT_THAT(conf2->GetProperties(), testing::SizeIs(0));

      // Configurations should differ and have unique PIDs.
      EXPECT_NE(conf, conf2);
      EXPECT_NE(conf->GetPid(), conf2->GetPid());
    }

    TEST_F(TestConfigurationAdminImpl, VerifyGetFactoryConfiguration)
    {
      auto bundleContext = GetFramework().GetBundleContext();
      auto fakeLogger = std::make_shared<FakeLogger>();
      ConfigurationAdminImpl configAdmin(bundleContext, fakeLogger);

      {
        const auto conf = configAdmin.GetFactoryConfiguration("factory", "instance1");
        ASSERT_TRUE(conf);
        EXPECT_EQ(conf->GetPid(), "factory~instance1");
        EXPECT_EQ(conf->GetFactoryPid(), "factory");
        auto props = conf->GetProperties();
        EXPECT_THAT(props, testing::SizeIs(0));
        props["foo"] = std::string{"bar"};
        EXPECT_NO_THROW(conf->Update(props));

        // Should return a different Configuration
        const auto conf2 = configAdmin.GetFactoryConfiguration("factory", "instance2");
        ASSERT_TRUE(conf2);
        EXPECT_EQ(conf2->GetPid(), "factory~instance2");
        EXPECT_EQ(conf2->GetFactoryPid(), "factory");
        EXPECT_THAT(conf2->GetProperties(), testing::SizeIs(0));

        EXPECT_NE(conf, conf2);
      }

      // Should still return the original Configuration even though it is now out-of-scope.
      const auto conf3 = configAdmin.GetFactoryConfiguration("factory", "instance1");
      ASSERT_TRUE(conf3);
      EXPECT_EQ(conf3->GetFactoryPid(), "factory");
      EXPECT_EQ(conf3->GetPid(), "factory~instance1");
      EXPECT_THAT(conf3->GetProperties(), testing::SizeIs(1));
    }

    TEST_F(TestConfigurationAdminImpl, VerifyListConfigurations)
    {
      auto bundleContext = GetFramework().GetBundleContext();
      auto fakeLogger = std::make_shared<FakeLogger>();
      ConfigurationAdminImpl configAdmin(bundleContext, fakeLogger);

      EXPECT_THROW(configAdmin.ListConfigurations(""), std::invalid_argument);
    }

    TEST_F(TestConfigurationAdminImpl, VerifyAddConfigurations)
    {
      auto bundleContext = GetFramework().GetBundleContext();
      auto fakeLogger = std::make_shared<FakeLogger>();
      ConfigurationAdminImpl configAdmin(bundleContext, fakeLogger);

      // Set up some existing Configurations
      const auto conf = configAdmin.GetConfiguration("test.pid");
      const auto conf2 = configAdmin.GetConfiguration("test.pid2");
      ASSERT_TRUE(conf);
      ASSERT_TRUE(conf2);
      auto props = conf2->GetProperties();
      props["foo"] = std::string{"bar"};
      EXPECT_NO_THROW(conf2->Update(props));

      std::vector<metadata::ConfigurationMetadata> configs;

      configs.push_back(metadata::ConfigurationMetadata("test.pid", AnyMap{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS}));

      AnyMap props2{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS};
      props2["foo"] = std::string{"baz"};
      configs.push_back(metadata::ConfigurationMetadata("test.pid2", props2));

      AnyMap props3{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS};
      props3["bar"] = std::string{"baz"};
      configs.push_back(metadata::ConfigurationMetadata("test.pid3", props3));

      auto result = configAdmin.AddConfigurations(std::move(configs));

      decltype(result) expected{ {"test.pid", 0ul, reinterpret_cast<std::uintptr_t>(conf.get())},
                                 {"test.pid2", 3ul, reinterpret_cast<std::uintptr_t>(conf2.get())} };

      ASSERT_THAT(result, testing::SizeIs(3));
      EXPECT_EQ(result[0], expected[0]);
      EXPECT_EQ(result[1], expected[1]);

      EXPECT_EQ(result[2].pid, "test.pid3");
      EXPECT_EQ(result[2].changeCount, 1ul);
      EXPECT_NE(result[2].configurationId, reinterpret_cast<std::uintptr_t>(conf.get()));
      EXPECT_NE(result[2].configurationId, reinterpret_cast<std::uintptr_t>(conf2.get()));

      props = conf2->GetProperties();
      EXPECT_THAT(props, AnyMapEquals(props2));

      auto conf3 = configAdmin.GetConfiguration("test.pid3");
      ASSERT_TRUE(conf3);
      props = conf3->GetProperties();
      EXPECT_THAT(props, AnyMapEquals(props3));
    }

    TEST_F(TestConfigurationAdminImpl, VerifyRemoveConfigurations)
    {
      auto bundleContext = GetFramework().GetBundleContext();
      auto fakeLogger = std::make_shared<FakeLogger>();
      ConfigurationAdminImpl configAdmin(bundleContext, fakeLogger);

      // Set up some existing Configurations
      const auto conf = configAdmin.GetConfiguration("test.pid");
      const auto conf2 = configAdmin.GetConfiguration("test.pid2");

      std::vector<metadata::ConfigurationMetadata> configs;

      configs.push_back(metadata::ConfigurationMetadata("test.pid", AnyMap{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS}));

      AnyMap props2{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS};
      props2["foo"] = std::string{"baz"};
      configs.push_back(metadata::ConfigurationMetadata("test.pid2", props2));

      AnyMap props3{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS};
      props3["bar"] = std::string{"baz"};
      configs.push_back(metadata::ConfigurationMetadata("test.pid3", props3));

      AnyMap props4{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS};
      props4["baz"] = std::string{"foo"};
      configs.push_back(metadata::ConfigurationMetadata("test.pid4", props4));

      configs.push_back(metadata::ConfigurationMetadata("test.pid5", AnyMap{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS}));

      auto result = configAdmin.AddConfigurations(std::move(configs));

      // The test.pid Configuration shouldn't have been modified, so shouldn't be removed.
      // Remove pid2, Update pid3, Remove & re-add pid4, so they too should be left alone.
      // Leave pid5 unchanged, and it alone should be Removed.
      const auto conf3 = configAdmin.GetConfiguration("test.pid3");
      auto conf4 = configAdmin.GetConfiguration("test.pid4");
      const auto conf5 = configAdmin.GetConfiguration("test.pid5");

      ASSERT_TRUE(conf);
      ASSERT_TRUE(conf2);
      ASSERT_TRUE(conf3);
      ASSERT_TRUE(conf4);
      ASSERT_TRUE(conf5);

      EXPECT_NO_THROW(conf2->Remove());
      props3["bar"] = std::string{"foo"};
      EXPECT_NO_THROW(conf3->Update(props3));
      EXPECT_NO_THROW(conf4->Remove());
      conf4 = configAdmin.GetConfiguration("test.pid4");

      EXPECT_NO_THROW(configAdmin.RemoveConfigurations(std::move(result)));

      EXPECT_EQ(conf, configAdmin.GetConfiguration("test.pid"));
      EXPECT_NE(conf2, configAdmin.GetConfiguration("test.pid2"));
      EXPECT_EQ(conf3, configAdmin.GetConfiguration("test.pid3"));
      EXPECT_EQ(conf4, configAdmin.GetConfiguration("test.pid4"));
      EXPECT_NE(conf5, configAdmin.GetConfiguration("test.pid5"));
    }

    TEST_F(TestConfigurationAdminImpl, VerifyManagedServiceNotification)
    {
      auto bundleContext = GetFramework().GetBundleContext();
      auto fakeLogger = std::make_shared<FakeLogger>();
      ConfigurationAdminImpl configAdmin(bundleContext, fakeLogger);

      // Set up an existing Configuration
      const auto conf = configAdmin.GetConfiguration("test.pid");
      ASSERT_TRUE(conf);
      auto props = conf->GetProperties();
      props["foo"] = std::string{"bar"};
      EXPECT_NO_THROW(conf->Update(props));

      AnyMap emptyProps{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS};

      std::mutex counterMutex;
      std::condition_variable counterCV;
      auto msCounter = 0u;
      auto ms2Counter = 0u;

      auto f = [&counterMutex, &counterCV, &msCounter]
      {
        {
          std::lock_guard<std::mutex> lk{counterMutex};
          ++msCounter;
        }
        counterCV.notify_one();
      };

      auto f2 = [&counterMutex, &counterCV, &ms2Counter]
      {
        {
          std::lock_guard<std::mutex> lk{counterMutex};
          ++ms2Counter;
        }
        counterCV.notify_one();
      };

      auto newProps = props;
      newProps["foo"] = std::string{"baz"};

      auto mockManagedService = std::make_shared<MockManagedService>();
      auto mockManagedService2 = std::make_shared<MockManagedService>();
      auto mockManagedService3 = std::make_shared<MockManagedService>();
      // setup expectations.
      EXPECT_CALL(*mockManagedService, Updated(AnyMapEquals(props)))
      .WillOnce(testing::InvokeWithoutArgs(f));
      EXPECT_CALL(*mockManagedService2, Updated(AnyMapEquals(emptyProps)))
      .WillOnce(testing::InvokeWithoutArgs(f2));
      EXPECT_CALL(*mockManagedService, Updated(AnyMapEquals(newProps)))
      .WillOnce(testing::InvokeWithoutArgs(f));
      EXPECT_CALL(*mockManagedService3, Updated(testing::_))
      .Times(0);

      // Ensure notification from original GetConfiguration has run.
      configAdmin.WaitForAllAsync();

      cppmicroservices::ServiceProperties ms1Props{{std::string("service.pid"), std::string("test.pid")}};
      cppmicroservices::ServiceProperties ms2Props{{std::string("component.name"), std::string("test.pid2")}};

      auto reg1 = bundleContext.RegisterService<cppmicroservices::service::cm::ManagedService>(mockManagedService, ms1Props);
      auto reg2 = bundleContext.RegisterService<cppmicroservices::service::cm::ManagedService>(mockManagedService2, ms2Props);
      auto reg3 = bundleContext.RegisterService<cppmicroservices::service::cm::ManagedService>(mockManagedService3);

      std::unique_lock<std::mutex> ul{counterMutex};
      auto invokedOnce = counterCV.wait_for(ul, std::chrono::seconds(10), [&msCounter] { return 1u == msCounter; });
      ul.unlock();

      EXPECT_TRUE(invokedOnce);
      EXPECT_NO_THROW(conf->Update(newProps));

      ul.lock();
      auto invokeComplete = counterCV.wait_for(ul, std::chrono::seconds(10), [&msCounter, &ms2Counter] { return 2u == msCounter && 1u == ms2Counter; });
      ul.unlock();

      EXPECT_TRUE(invokeComplete);

      reg1.Unregister();
      reg2.Unregister();
      reg3.Unregister();

      configAdmin.WaitForAllAsync();
    }

    TEST_F(TestConfigurationAdminImpl, VerifyManagedServiceFactoryNotification)
    {
      auto bundleContext = GetFramework().GetBundleContext();
      auto fakeLogger = std::make_shared<FakeLogger>();
      ConfigurationAdminImpl configAdmin(bundleContext, fakeLogger);

      // Set up an existing Configuration
      const auto conf = configAdmin.GetConfiguration("factory2~instance1");
      ASSERT_TRUE(conf);
      auto props = conf->GetProperties();
      props["foo"] = std::string{"bar"};
      EXPECT_NO_THROW(conf->Update(props));

      AnyMap emptyProps{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS};

      std::mutex counterMutex;
      std::condition_variable counterCV;
      auto msCounter = 0u;
      auto ms2Counter = 0u;

      auto f = [&counterMutex, &counterCV, &msCounter]
      {
        {
          std::lock_guard<std::mutex> lk{counterMutex};
          ++msCounter;
        }
        counterCV.notify_one();
      };

      auto f2 = [&counterMutex, &counterCV, &ms2Counter]
      {
        {
          std::lock_guard<std::mutex> lk{counterMutex};
          ++ms2Counter;
        }
        counterCV.notify_one();
      };

      auto newProps = props;
      newProps["foo"] = std::string{"baz"};

      auto mockManagedServiceFactory = std::make_shared<MockManagedServiceFactory>();
      auto mockManagedServiceFactory2 = std::make_shared<MockManagedServiceFactory>();
      auto mockManagedServiceFactory3 = std::make_shared<MockManagedServiceFactory>();
      // setup expectations.
      EXPECT_CALL(*mockManagedServiceFactory2, Updated(std::string{"factory2~instance1"}, AnyMapEquals(props)))
      .WillOnce(testing::InvokeWithoutArgs(f2));
      EXPECT_CALL(*mockManagedServiceFactory, Updated(std::string{"factory~instance1"}, AnyMapEquals(emptyProps)))
      .WillOnce(testing::InvokeWithoutArgs(f));
      EXPECT_CALL(*mockManagedServiceFactory, Updated(std::string{"factory~instance2"}, AnyMapEquals(emptyProps)))
      .WillOnce(testing::InvokeWithoutArgs(f));
      EXPECT_CALL(*mockManagedServiceFactory2, Updated(std::string{"factory2~instance1"}, AnyMapEquals(newProps)))
      .WillOnce(testing::InvokeWithoutArgs(f2));
      EXPECT_CALL(*mockManagedServiceFactory, Removed(std::string{"factory~instance1"}))
      .WillOnce(testing::InvokeWithoutArgs(f));
      EXPECT_CALL(*mockManagedServiceFactory3, Updated(testing::_, testing::_))
      .Times(0);
      EXPECT_CALL(*mockManagedServiceFactory3, Removed(testing::_))
      .Times(0);

      // Ensure notification from original GetConfiguration has run.
      configAdmin.WaitForAllAsync();

      AnyMap pidProp{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS};
      pidProp["pid"] = std::string("factory");
      AnyMap nameProp{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS};
      nameProp["name"] = std::string("factory2");
      cppmicroservices::ServiceProperties ms1Props{{std::string("service"), pidProp}};
      cppmicroservices::ServiceProperties ms2Props{{std::string("component"), nameProp}};

      auto reg1 = bundleContext.RegisterService<cppmicroservices::service::cm::ManagedServiceFactory>(mockManagedServiceFactory, ms1Props);
      auto reg2 = bundleContext.RegisterService<cppmicroservices::service::cm::ManagedServiceFactory>(mockManagedServiceFactory2, ms2Props);
      auto reg3 = bundleContext.RegisterService<cppmicroservices::service::cm::ManagedServiceFactory>(mockManagedServiceFactory3);

      std::unique_lock<std::mutex> ul{counterMutex};
      auto factory2InvokedOnce = counterCV.wait_for(ul, std::chrono::seconds(10), [&ms2Counter] { return 1u == ms2Counter; });
      ul.unlock();

      EXPECT_TRUE(factory2InvokedOnce);
      configAdmin.WaitForAllAsync();

      auto conf2 = configAdmin.GetConfiguration("factory~instance1");
      auto conf3 = configAdmin.GetConfiguration("factory~instance2");
      ASSERT_TRUE(conf2);
      ASSERT_TRUE(conf3);

      ul.lock();
      auto factory1InvokedTwice = counterCV.wait_for(ul, std::chrono::seconds(10), [&msCounter] { return 2u == msCounter; });
      ul.unlock();

      EXPECT_TRUE(factory1InvokedTwice);
      EXPECT_NO_THROW(conf->Update(newProps));
      EXPECT_NO_THROW(conf2->Remove());

      ul.lock();
      auto invokeComplete = counterCV.wait_for(ul, std::chrono::seconds(10), [&msCounter, &ms2Counter] { return 3u == msCounter && 2u == ms2Counter; });
      ul.unlock();

      EXPECT_TRUE(invokeComplete);

      reg1.Unregister();
      reg2.Unregister();
      reg3.Unregister();

      configAdmin.WaitForAllAsync();
    }

    TEST_F(TestConfigurationAdminImpl, VerifyConfigAdminStartupShutdownNotification)
    {
      auto bundleContext = GetFramework().GetBundleContext();
      auto fakeLogger = std::make_shared<FakeLogger>();

      AnyMap emptyProps{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS};

      std::mutex counterMutex;
      std::condition_variable counterCV;
      auto msCounter = 0u;
      auto msfCounter = 0u;

      auto f = [&counterMutex, &counterCV, &msCounter]
      {
        {
          std::lock_guard<std::mutex> lk{counterMutex};
          ++msCounter;
        }
        counterCV.notify_one();
      };

      auto f2 = [&counterMutex, &counterCV, &msfCounter]
      {
        {
          std::lock_guard<std::mutex> lk{counterMutex};
          ++msfCounter;
        }
        counterCV.notify_one();
      };

      auto mockManagedService = std::make_shared<MockManagedService>();
      auto mockManagedService2 = std::make_shared<MockManagedService>();
      auto mockManagedServiceFactory = std::make_shared<MockManagedServiceFactory>();
      auto mockManagedServiceFactory2 = std::make_shared<MockManagedServiceFactory>();
      // setup expectations.
      EXPECT_CALL(*mockManagedService, Updated(AnyMapEquals(emptyProps)))
      .Times(2).WillRepeatedly(testing::InvokeWithoutArgs(f));
      EXPECT_CALL(*mockManagedServiceFactory, Updated(std::string{"factory~instance1"}, AnyMapEquals(emptyProps)))
      .WillOnce(testing::InvokeWithoutArgs(f2));
      EXPECT_CALL(*mockManagedServiceFactory, Removed(std::string{"factory~instance1"}))
      .WillOnce(testing::InvokeWithoutArgs(f2));
      EXPECT_CALL(*mockManagedService2, Updated(testing::_))
      .Times(0);
      EXPECT_CALL(*mockManagedServiceFactory2, Updated(testing::_, testing::_))
      .Times(0);
      EXPECT_CALL(*mockManagedServiceFactory2, Removed(testing::_))
      .Times(0);

      cppmicroservices::ServiceProperties msProps{{std::string("service.pid"), std::string("test.pid")}};
      cppmicroservices::ServiceProperties msfProps{{std::string("service.pid"), std::string("factory")}};

      auto reg1 = bundleContext.RegisterService<cppmicroservices::service::cm::ManagedServiceFactory>(mockManagedServiceFactory, msfProps);
      auto reg2 = bundleContext.RegisterService<cppmicroservices::service::cm::ManagedServiceFactory>(mockManagedServiceFactory2);
      auto reg3 = bundleContext.RegisterService<cppmicroservices::service::cm::ManagedService>(mockManagedService, msProps);
      auto reg4 = bundleContext.RegisterService<cppmicroservices::service::cm::ManagedService>(mockManagedService2);

      {
        ConfigurationAdminImpl configAdmin(bundleContext, fakeLogger);

        std::unique_lock<std::mutex> ul{counterMutex};
        auto invokedOnce = counterCV.wait_for(ul, std::chrono::seconds(10), [&msCounter] { return 1u == msCounter; });
        ul.unlock();

        EXPECT_TRUE(invokedOnce);
        configAdmin.WaitForAllAsync();

        auto conf = configAdmin.GetConfiguration("factory~instance1");
        ASSERT_TRUE(conf);

        ul.lock();
        auto factoryInvokedOnce = counterCV.wait_for(ul, std::chrono::seconds(10), [&msfCounter] { return 1u == msfCounter; });
        ul.unlock();

        EXPECT_TRUE(factoryInvokedOnce);
      }

      std::unique_lock<std::mutex> ul{counterMutex};
      auto invokeComplete = counterCV.wait_for(ul, std::chrono::seconds(10), [&msCounter, &msfCounter] { return 2u == msCounter && 2u == msfCounter; });
      ul.unlock();

      EXPECT_TRUE(invokeComplete);

      reg1.Unregister();
      reg2.Unregister();
      reg3.Unregister();
      reg4.Unregister();
    }

    TEST_F(TestConfigurationAdminImpl, VerifyManagedServiceExceptionsAreLogged)
    {
      using cppmicroservices::logservice::SeverityLevel;
      using cppmicroservices::service::cm::ConfigurationException;

      auto bundleContext = GetFramework().GetBundleContext();
      auto mockLogger = std::make_shared<MockLogger>();

      // set logging expectations
      auto ConfigurationExceptionThrownByManagedService = testing::AllOf(
        testing::HasSubstr("ConfigurationException thrown by ManagedService with PID test.pid whilst being Updated with new properties"),
        testing::HasSubstr("The property which caused this error was 'foo'"),
        testing::HasSubstr("Exception reason: A reason"));
      auto ExceptionThrownByManagedService = testing::AllOf(
        testing::HasSubstr("Exception thrown by ManagedService with PID test.pid whilst being Updated with new properties"),
        testing::HasSubstr("Exception: An exception"));
      auto UnknownExceptionThrownByManagedService =
        testing::HasSubstr("Unknown exception thrown by ManagedService with PID test.pid whilst being Updated with new properties");

      auto ExceptionThrownByManagedServiceFactoryDuringUpdate = testing::AllOf(
        testing::HasSubstr("Exception thrown by ManagedServiceFactory with PID factory~"),
        testing::HasSubstr("whilst being Updated with new properties"),
        testing::HasSubstr("Exception: An exception"));
      auto UnknownExceptionThrownByManagedServiceFactoryDuringUpdate = testing::AllOf(
        testing::HasSubstr("Unknown exception thrown by ManagedServiceFactory with PID factory~"),
        testing::HasSubstr("whilst being Updated with new properties"));

      auto ExceptionThrownByManagedServiceFactoryDuringRemoval = testing::AllOf(
        testing::HasSubstr("Exception thrown by ManagedServiceFactory with PID factory~"),
        testing::HasSubstr("whilst being notified of Configuration Removal."),
        testing::HasSubstr("Exception: An exception"));
      auto UnknownExceptionThrownByManagedServiceFactoryDuringRemoval = testing::AllOf(
        testing::HasSubstr("Unknown exception thrown by ManagedServiceFactory with PID factory~"),
        testing::HasSubstr("whilst being notified of Configuration Removal."));

      EXPECT_CALL(*mockLogger, Log(SeverityLevel::LOG_DEBUG, testing::_)).Times(testing::AtLeast(1));
      EXPECT_CALL(*mockLogger, Log(SeverityLevel::LOG_ERROR, ConfigurationExceptionThrownByManagedService)).Times(1);
      EXPECT_CALL(*mockLogger, Log(SeverityLevel::LOG_ERROR, ExceptionThrownByManagedService)).Times(1);
      EXPECT_CALL(*mockLogger, Log(SeverityLevel::LOG_ERROR, UnknownExceptionThrownByManagedService)).Times(1);
      EXPECT_CALL(*mockLogger, Log(SeverityLevel::LOG_ERROR, ExceptionThrownByManagedServiceFactoryDuringUpdate)).Times(1);
      EXPECT_CALL(*mockLogger, Log(SeverityLevel::LOG_ERROR, UnknownExceptionThrownByManagedServiceFactoryDuringUpdate)).Times(1);
      EXPECT_CALL(*mockLogger, Log(SeverityLevel::LOG_ERROR, ExceptionThrownByManagedServiceFactoryDuringRemoval)).Times(1);
      EXPECT_CALL(*mockLogger, Log(SeverityLevel::LOG_ERROR, UnknownExceptionThrownByManagedServiceFactoryDuringRemoval)).Times(1);

      ConfigurationAdminImpl configAdmin(bundleContext, mockLogger);

      // Set up an existing Configuration
      const auto conf = configAdmin.GetConfiguration("test.pid");
      ASSERT_TRUE(conf);

      AnyMap emptyProps{AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS};

      std::mutex counterMutex;
      std::condition_variable counterCV;
      auto msCounter = 0u;
      auto msfCounter = 0u;

      auto f = [&counterMutex, &counterCV, &msCounter]
      {
        {
          std::lock_guard<std::mutex> lk{counterMutex};
          ++msCounter;
        }
        counterCV.notify_one();
      };

      auto f2 = [&counterMutex, &counterCV, &msfCounter]
      {
        {
          std::lock_guard<std::mutex> lk{counterMutex};
          ++msfCounter;
        }
        counterCV.notify_one();
      };

      auto mockManagedService = std::make_shared<MockManagedService>();
      auto mockManagedServiceFactory = std::make_shared<MockManagedServiceFactory>();
      // setup expectations.
      EXPECT_CALL(*mockManagedService, Updated(AnyMapEquals(emptyProps)))
      .WillOnce(testing::DoAll(testing::InvokeWithoutArgs(f), testing::Throw(ConfigurationException("A reason", "foo"))))
      .WillOnce(testing::DoAll(testing::InvokeWithoutArgs(f), testing::Throw(std::runtime_error("An exception"))))
      .WillOnce(testing::DoAll(testing::InvokeWithoutArgs(f), testing::Throw(42)));

      EXPECT_CALL(*mockManagedServiceFactory, Updated(testing::HasSubstr("factory~"), AnyMapEquals(emptyProps)))
      .WillOnce(testing::DoAll(testing::InvokeWithoutArgs(f2), testing::Throw(std::runtime_error("An exception"))))
      .WillOnce(testing::DoAll(testing::InvokeWithoutArgs(f2), testing::Throw(42)));

      EXPECT_CALL(*mockManagedServiceFactory, Removed(testing::HasSubstr("factory~")))
      .WillOnce(testing::DoAll(testing::InvokeWithoutArgs(f2), testing::Throw(std::runtime_error("An exception"))))
      .WillOnce(testing::DoAll(testing::InvokeWithoutArgs(f2), testing::Throw(42)));

      // Ensure notification from original GetConfiguration has run.
      configAdmin.WaitForAllAsync();

      cppmicroservices::ServiceProperties msProps{{std::string("service.pid"), std::string("test.pid")}};
      cppmicroservices::ServiceProperties msfProps{{std::string("service.pid"), std::string("factory")}};

      auto reg1 = bundleContext.RegisterService<cppmicroservices::service::cm::ManagedServiceFactory>(mockManagedServiceFactory, msfProps);
      auto reg2 = bundleContext.RegisterService<cppmicroservices::service::cm::ManagedService>(mockManagedService, msProps);

      std::unique_lock<std::mutex> ul{counterMutex};
      auto invokedOnce = counterCV.wait_for(ul, std::chrono::seconds(10), [&msCounter] { return 1u == msCounter; });
      ul.unlock();

      EXPECT_TRUE(invokedOnce);
      configAdmin.WaitForAllAsync();

      EXPECT_NO_THROW(conf->Update(emptyProps));
      EXPECT_NO_THROW(conf->Remove());

      auto factoryConf = configAdmin.CreateFactoryConfiguration("factory");
      auto factoryConf2 = configAdmin.CreateFactoryConfiguration("factory");

      ul.lock();
      auto factoryInvokedTwice = counterCV.wait_for(ul, std::chrono::seconds(10), [&msfCounter] { return 2u == msfCounter; });
      US_UNUSED(factoryInvokedTwice);
      ul.unlock();

      EXPECT_NO_THROW(factoryConf->Remove());
      EXPECT_NO_THROW(factoryConf2->Remove());

      ul.lock();
      auto invokeComplete = counterCV.wait_for(ul, std::chrono::seconds(10), [&msCounter, &msfCounter] { return 3u == msCounter && 4u == msfCounter; });
      ul.unlock();

      EXPECT_TRUE(invokeComplete);

      reg1.Unregister();
      reg2.Unregister();

      configAdmin.WaitForAllAsync();
    }
  }
}
