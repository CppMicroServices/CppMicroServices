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

#include "gtest/gtest.h"
#include "TestFixture.hpp"

#include "TestInterfaces/Interfaces.hpp"

namespace test {

/**
* Test the component instance is created with default properties when only the 
* configuration policy is specified (no PID specified).
*/
TEST_F(tServiceComponent, testWithoutPID) // DS_CAI_FTC_14
{
  // Start the test bundle containing the component name.
  std::string componentName = "sample::ServiceComponentCA14";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA14");

  // Use DS runtime service to validate the component state.
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
    GetComponentConfigs(testBundle, componentName, compDescDTO);

  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "Ignore policy component state should be ACTIVE";

  auto configAdminService =
    GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(configAdminService)
    << "GetService failed for ConfigurationAdmin.";

  // Create configuration object, try to update property
  // and property update should be ignored.
  auto configuration = configAdminService->GetConfiguration(componentName);
  auto configInstance = configuration->GetPid();

  // Confirm configuratino object presented and check component state.
  compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "ACTIVE is expected since configuration object is created.";

  // GetService to make component active.
  auto instance = GetInstance<test::CAInterface>();
  ASSERT_TRUE(instance) << "GetService failed for CAInterface";

  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  const std::string instanceId{ "instance1" };
  props["uniqueProp"] = instanceId;
  auto fut = configuration->Update(props);
  fut.get();
  // Confirm component instance was created with the properties has default value.
  EXPECT_TRUE(instance->GetProperties().empty())
    << "Property instance should only have default value.";
} // end of testWithoutPID

/**
* Test the component instance is created with default properties when only the 
* configuration PID is specified (no policy specified).
*/
TEST_F(tServiceComponent, testWithoutConfigPolicy) // DS_CAI_FTC_15
{
  // Start the test bundle containing the component name.
  std::string componentName = "sample::ServiceComponentCA15";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA15");

  // Use DS runtime service to validate the component state.
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
    GetComponentConfigs(testBundle, componentName, compDescDTO);

  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "Ignore policy component state should be ACTIVE";

  auto configAdminService =
    GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(configAdminService)
    << "GetService failed for ConfigurationAdmin.";

  // Create configuration object, try to update property
  // and property update should be ignored.
  auto configuration = configAdminService->GetConfiguration(componentName);
  auto configInstance = configuration->GetPid();

  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  const std::string instanceId{ "instance1" };
  props["uniqueProp"] = instanceId;
  auto fut = configuration->Update(props);
  fut.get();
  // Confirm configuratino object presented and check component state.
  compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "ACTIVE is expected since configuration object is created.";

  // GetService to make component active.
  auto instance = GetInstance<test::CAInterface>();
  ASSERT_TRUE(instance) << "GetService failed for CAInterface";

  // Confirm component instance was created with the properties has default value.
  EXPECT_TRUE(instance->GetProperties().empty())
    << "Property instance should only have default value.";
} // end of testWithoutConfigPolicy
} // end of test namespace