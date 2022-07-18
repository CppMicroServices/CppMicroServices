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

#include <ostream>
#include <regex>
#include <string>

#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>
#include "LogServiceImpl.hpp"

namespace ls = cppmicroservices::logservice;

static const std::string sinkFormat = "[%T] [%P:%t] %n (%^%l%$): %v";
static const std::string log_preamble(
  "\\[([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})\\] "
  "\\[([0-9]{1,9}):([0-9]{1,9})\\] cppmicroservices::testing::logservice "
  "\\((debug|trace|info|warning|error)\\): ");

namespace cppmicroservices {
namespace scrimpl {

class ServiceDependencyErrorLoggingTest : public ::testing::Test
{

public:
  bool ContainsRegex(const std::string& regex)
  {
    std::string text = oss.str();
    std::smatch m;
    bool found = std::regex_search(text, m, std::regex(regex));
    oss.str("");
    return found;
  }

  std::shared_ptr<ls::LogServiceImpl> GetLogger() { return _impl; }
  std::ostringstream& GetStream() { return oss; }

protected:
  ServiceDependencyErrorLoggingTest()
      : framework(cppmicroservices::FrameworkFactory().NewFramework())
  {
    _impl = std::make_shared<ls::LogServiceImpl>(
      "cppmicroservices::testing::logservice");
  }

  virtual ~ServiceDependencyErrorLoggingTest() = default;

  virtual void SetUp()
  {
    framework.Start();
    _sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    _sink->set_pattern(sinkFormat);
    _impl->AddSink(_sink);
  }

  virtual void TearDown()
  {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
    _sink.reset();
    _impl.reset();
  }

 cppmicroservices::Framework& GetFramework() { return framework; }

private:
  std::ostringstream oss;
  spdlog::sink_ptr _sink;
  std::shared_ptr<ls::LogServiceImpl> _impl;

public:
  cppmicroservices::Framework framework;
};

TEST_F(ServiceDependencyErrorLoggingTest, ProperLoggerUsage)
{
  auto logger = GetLogger();
  logger->Log(ls::SeverityLevel::LOG_DEBUG, "Bonjour!");
  EXPECT_TRUE(ContainsRegex(log_preamble + "Bonjour!"));
}

TEST_F(ServiceDependencyErrorLoggingTest, TestServiceDependencyLDAPFilter)
{
  auto serviceImpllogger = GetLogger();
 
  GetFramework().GetBundleContext()
    .RegisterService<cppmicroservices::logservice::LogService>(
      serviceImpllogger);

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
    framework.GetBundleContext(), serviceImpllogger, asyncWorkService);
  auto managers =
    std::make_shared<std::vector<std::shared_ptr<ComponentManager>>>();

  {
    auto refmockMetadata = std::make_shared<metadata::ComponentMetadata>();
    refmockMetadata->serviceMetadata.interfaces = {
      us_service_interface_iid<dummy::Reference1>()
    };
    // Test for wrong LDAP filter values, actual LDAP string should be "abc" as per ServiceImpl reference properties
    refmockMetadata->properties.insert(
      std::make_pair("scheme", Any(std::string("abcd"))));
    auto fakeCompConfig =
      std::make_shared<SingletonComponentConfigurationImpl>(mockMetadata,
                                                            framework,
                                                            mockRegistry,
                                                            serviceImpllogger,
                                                            notifier,
                                                            managers);
    EXPECT_EQ(fakeCompConfig->GetConfigState(),
              ComponentState::UNSATISFIED_REFERENCE);
    fakeCompConfig->Initialize();
    auto serviceRefFakeCompConfig =
      std::make_shared<SingletonComponentConfigurationImpl>(refmockMetadata,
                                                            framework,
                                                            mockRegistry,
                                                            serviceImpllogger,
                                                            notifier,
                                                            managers);
    serviceRefFakeCompConfig->Initialize();
    EXPECT_TRUE(ContainsRegex("reference LDAP filter doesn't match"));
    fakeCompConfig->Deactivate();
    fakeCompConfig->Stop();
    serviceRefFakeCompConfig->Deactivate();
    serviceRefFakeCompConfig->Stop();
  }

  {
    auto refmockMetadata = std::make_shared<metadata::ComponentMetadata>();
    refmockMetadata->serviceMetadata.interfaces = {
      us_service_interface_iid<dummy::Reference1>()
    };
    // Test for correct LDAP filter values
    refmockMetadata->properties.insert(
      std::make_pair("scheme", Any(std::string("abc"))));
    auto fakeCompConfig =
      std::make_shared<SingletonComponentConfigurationImpl>(mockMetadata,
                                                            framework,
                                                            mockRegistry,
                                                            serviceImpllogger,
                                                            notifier,
                                                            managers);
    EXPECT_EQ(fakeCompConfig->GetConfigState(),
              ComponentState::UNSATISFIED_REFERENCE);
    fakeCompConfig->Initialize();
    auto serviceRefFakeCompConfig =
      std::make_shared<SingletonComponentConfigurationImpl>(refmockMetadata,
                                                            framework,
                                                            mockRegistry,
                                                            serviceImpllogger,
                                                            notifier,
                                                            managers);
    serviceRefFakeCompConfig->Initialize();
    EXPECT_FALSE(ContainsRegex("reference LDAP filter doesn't match"));
    fakeCompConfig->Deactivate();
    fakeCompConfig->Stop();
    serviceRefFakeCompConfig->Deactivate();
    serviceRefFakeCompConfig->Stop();
  }
}
}
}
