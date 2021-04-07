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
* Test the component instance is constructed without the configuration object is
* present and the component should be constructed with no properties.
*/
TEST_F(tServiceComponent, testOptionalConfigPolicyWithoutConfigObj) // DS_CAI_FTC_5
{
  // Start the test bundle containing the component name.
  std::string componentName           = "sample::ServiceComponentCA05";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA05");

  // Use DS runtime service to validate the component state
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "The state should be active once the component instance is constructed.";

  // Request a service reference to the new component instance. This will
  // cause DS to construct the instance with the updated properties.
  auto instance = GetInstance<test::CAInterface>();
  ASSERT_TRUE(instance) << "GetService failed for CAInterface";

  // Confirm component instance was created with the properties has default value
  auto instanceProps = instance->GetProperties();
  EXPECT_TRUE(instanceProps.empty())
    << "Property instance should be empty";
} // end of testOptionalConfigPolicyWithoutConfigObj

/**
* Test the component instance is constructed with the configuration object is present
* and the component should be constructed with the correct properties.
*/
TEST_F(tServiceComponent, testOptionalConfigPolicyWithConfigObj) // DS_CAI_FTC_5
{
  // Start the test bundle containing the component name.
  std::string componentName           = "sample::ServiceComponentCA05a";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA05a");

  // Use DS runtime service to validate the component description and
  // Verify that DS is finished creating the component data structures.
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
    GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "Component state should be ACTIVE";

  // Get a service reference to ConfigAdmin to create the component instance.
  auto configAdminService =
    GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

  //Create configuration object
  auto configuration = configAdminService->GetConfiguration(componentName);
  auto configInstance = configuration->GetPid();

  // Created the configuration object on which the component is configured
  // but it created it with no properties. Update the properties before
  // instantiating the component instance.
  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  const std::string instanceId{ "instance" };
  props["uniqueProp"] = instanceId;
  configuration->Update(props);

  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "Component instance state should be ACTIVE";

  // Request a service reference to the new component instance. This will
  // cause DS to construct the instance with the updated properties.
  auto instance = GetInstance<test::CAInterface>();
  ASSERT_TRUE(instance) << "GetService failed for CAInterface";

  //Confirm component instance was created with the correct properties
  auto instanceProps = instance->GetProperties();
  auto uniqueProp = instanceProps.find("uniqueProp");

  ASSERT_TRUE(uniqueProp != instanceProps.end())
    << " not found in constructed instance";
  EXPECT_EQ(uniqueProp->second, instanceId);
} // end of testOptionalConfigPolicyWithConfigObj

TEST_F(tServiceComponent, testRequiredConfigPolicy) // DS_CAI_FTC_6
{
  // Start the test bundle containing the component name.
  std::string componentName           = "sample::ServiceComponentCA20";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA20");

  // Use DS runtime service to validate the component state
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
    GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state,
            scr::dto::ComponentState::UNSATISFIED_REFERENCE)
    << "UNSATISFIED_REFERENCE is expected because configuration object is not "
       "created.";

  // Get a service reference to ConfigAdmin to create the component instance.
  auto configAdminService =
    GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

  // Create configuration object and update property
  auto configuration  = configAdminService->GetConfiguration(componentName);
  auto configInstance = configuration->GetPid();

  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  const std::string instanceId{ "instance1" };
  props["uniqueProp"] = instanceId;
  configuration->Update(props);

  // Confirm configuration object presented and component is satisfied.
  compConfigs =
      GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
    << "SATISFIED is exepected since configuration object is created.";

  // Request a service reference to the new component instance. This will
  // cause DS to construct the instance with the updated properties.
  auto instance = GetInstance<test::CAInterface>();
  ASSERT_TRUE(instance) << "GetService failed for CAInterface";

  // Confirm component instance was created with the correct properties
  auto instanceProps = instance->GetProperties();
  auto uniqueProp    = instanceProps.find("uniqueProp");

  ASSERT_TRUE(uniqueProp != instanceProps.end())
    << "uniqueProp not found in constructed instance";
  EXPECT_EQ(uniqueProp->second, instanceId);
} // end of testRequiredConfigPolicy

/**
* Test the component instance is created with default properties without the configuration object.
*/
TEST_F(tServiceComponent, testIgnoreConfigPolicyWithoutConfigObj) // DS_CAI_FTC_7
{
  // Start the test bundle containing the component name.
  std::string           componentName = "sample::ServiceComponentCA07";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA07");

  // Use DS runtime service to validate the component state
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
    GetComponentConfigs(testBundle, componentName, compDescDTO);

  // Confirm configuration object presented and component is ACTIVE
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "Ignore policy component state should be ACTIVE";

  // Request a service reference to the new component instance. This will
  // cause DS to construct the instance with the updated properties.
  auto instance = GetInstance<test::CAInterface>();
  ASSERT_TRUE(instance) << "GetService failed for CAInterface";

  // Confirm component instance was created with the properties has default value
  auto instanceProps = instance->GetProperties();
  EXPECT_TRUE(instanceProps.empty())
    << "Property instance should be empty";
} // end of testIgnoreConfigPolicyWithoutConfigObj

/**
* Test the component instance is created with default properties not from the configuration object.
*/
TEST_F(tServiceComponent, testIgnoreConfigPolicyWithConfigObj) // DS_CAI_FTC_7
{
  // Start the test bundle containing the component name.
  std::string componentName = "sample::ServiceComponentCA07a";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA07a");

  // Use DS runtime service to validate the component state
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
    GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "Ignore policy component state should be ACTIVE";

  // Get a service reference to ConfigAdmin to create the component instance.
  auto configAdminService =
    GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

  // Request a service reference to the new component instance. This will
  // cause DS to construct the instance with the updated properties.
  auto instance = GetInstance<test::CAInterface>();
  ASSERT_TRUE(instance) << "GetService failed for CAInterface";

  // Confirm component instance was created with the properties has default value
  auto instanceProps = instance->GetProperties();
  EXPECT_TRUE(instanceProps.empty())
    << "Property instance should be empty";
} // end of testIgnoreConfigPolicyWithConfigObj

} // end of test namespace
