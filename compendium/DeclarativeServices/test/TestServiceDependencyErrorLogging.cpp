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

#include <gtest/gtest.h>

#include "../src/manager/ComponentConfigurationImpl.hpp"
#include "../src/manager/ReferenceManager.hpp"
#include "../src/manager/SingletonComponentConfiguration.hpp"
#include "../src/metadata/ComponentMetadata.hpp"
#include "Mocks.hpp"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/ServiceInterface.h"

#include <string>

namespace cppmicroservices {
namespace scrimpl {

class CustomLogger : public FakeLogger
{
  std::string msg;

public:
  using FakeLogger::Log;
  void Log(cppmicroservices::logservice::SeverityLevel,
           const std::string& message) override
  {
    msg = message;
  }

  std::string GetLoggerMessage() { return msg; }
};

class ServiceDependencyErrorLoggingTest : public ::testing::Test
{

public:
  std::shared_ptr<CustomLogger> GetLogger() { return logger_; }

protected:
  ServiceDependencyErrorLoggingTest()
      : framework(cppmicroservices::FrameworkFactory().NewFramework())
  {
    logger_ = std::make_shared<CustomLogger>(); 
  }

  virtual ~ServiceDependencyErrorLoggingTest() = default;

  virtual void SetUp()
  {
    framework.Start();
  }

  virtual void TearDown()
  {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }

 cppmicroservices::Framework& GetFramework() { return framework; }

private:
  std::shared_ptr<CustomLogger> logger_;


public:
  cppmicroservices::Framework framework;
};


TEST_F(ServiceDependencyErrorLoggingTest, TestServiceDependencyLDAPFilter)
{
  auto customLogger = GetLogger();
  GetFramework().GetBundleContext()
    .RegisterService<cppmicroservices::logservice::LogService>(customLogger);

  auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
  mockMetadata->serviceMetadata.interfaces = {
    us_service_interface_iid<dummy::ServiceImpl>()
  };

  // reference meta data for ServiceImpl
  metadata::ReferenceMetadata rm1;
  rm1.name = "Reference1";
  rm1.interfaceName = "cppmicroservices::scrimpl::dummy::Reference1";
  rm1.target = "(scheme=abc)";
  mockMetadata->refsMetadata.push_back(rm1);

  auto mockRegistry = std::make_shared<MockComponentRegistry>();
  auto logger = std::make_shared<SCRLogger>(framework.GetBundleContext());
  auto asyncWorkService =
    std::make_shared<cppmicroservices::scrimpl::SCRAsyncWorkService>(
      framework.GetBundleContext(), logger);
  auto notifier = std::make_shared<ConfigurationNotifier>(
    framework.GetBundleContext(), customLogger, asyncWorkService);
  auto managers =
    std::make_shared<std::vector<std::shared_ptr<ComponentManager>>>();

  {
    auto refmockMetadata = std::make_shared<metadata::ComponentMetadata>();
    refmockMetadata->serviceMetadata.interfaces = {
      us_service_interface_iid<dummy::Reference1>()
    };
    // create wrong LDAP filter value, actual LDAP string should be "abc" as per ServiceImpl reference properties
    refmockMetadata->properties.insert(
      std::make_pair("scheme", Any(std::string("abcd"))));
    auto fakeCompConfig =
      std::make_shared<SingletonComponentConfigurationImpl>(mockMetadata,
                                                            framework,
                                                            mockRegistry, customLogger,
                                                            notifier,
                                                            managers);
    EXPECT_EQ(fakeCompConfig->GetConfigState(),
              ComponentState::UNSATISFIED_REFERENCE);
    fakeCompConfig->Initialize();
    auto serviceRefFakeCompConfig =
      std::make_shared<SingletonComponentConfigurationImpl>(refmockMetadata,
                                                            framework,
                                                            mockRegistry,
                                                            customLogger,
                                                            notifier,
                                                            managers);
    serviceRefFakeCompConfig->Initialize();
    using ::testing::ContainsRegex;
    EXPECT_THAT(
      customLogger->GetLoggerMessage(), ContainsRegex("doesn't match service reference properties"));
    fakeCompConfig->Deactivate();
    fakeCompConfig->Stop();
    serviceRefFakeCompConfig->Deactivate();
    serviceRefFakeCompConfig->Stop();
  }
}
}
}
