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

#include "cppmicroservices/servicecomponent/detail/ComponentInstanceImpl.hpp"

namespace test {

TEST_F(tServiceComponent, testOptionalConfigPolicyImmediateTrue) // DS_CAI_FTC_5
{
  // Start the test bundle containing the component name.
  std::string componentName           = "sample::ServiceComponentCA5";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA5");

  // Use DS runtime service to validate the component state
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "The state should be active once the component instance is constructed.";

  // Get a service reference to ConfigAdmin to create the component instance.
  auto configAdminService =
    GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

  //Create configuration object
  auto configuration  = configAdminService->GetConfiguration(componentName);
  auto configInstance = configuration->GetPid();

  // Created the configuration object on which the component is configured
  // but it created it with no properties. Update the properties before
  // instantiating the component instance.
  std::string uniqueProp[3] = { "uniqueProp1", "uniqueProp2", "uniqueProp3" };
  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  const std::string instance[3] = { "instance1", "instance2", "instance3" };
  cppmicroservices::AnyMap instanceProps(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);

  // Repeat a few time to check the expected behavior
  int i = 0;
  while (i < 3)
  {
    // The Update properties sends an asynchronous request to DS to
    // update the properties. Must wait until DS is finished before we can continue.
    // Use DS runtime service to validate the component properties. May take more
    // than one try.

    props[uniqueProp[i]] = instance[i];
    configuration->Update(props);
    auto result = RepeatTaskUntilOrTimeout(
      [&compDescDTO, &compConfigs, this, &testBundle, &configInstance]() {
        compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(
          testBundle, configInstance);
        if (compDescDTO.name != "") {
          compConfigs =
            this->dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
        }
      },
      [&compConfigs, &instance, &uniqueProp, &i]() -> bool {
        if (compConfigs.size() == 1) {
          auto properties = compConfigs.at(0).properties;
          auto id = properties.find(uniqueProp[i]);
          if (id != properties.end()) {
            return (id->second == instance[i]);
          }
        }
        return false;
      });

      ASSERT_TRUE(result) << "Timed out waiting for Update Configuration"
                             "to complete.";
      EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
      EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
        << "Component instance state should be ACTIVE";

      //Request a service reference to the new component instance. This will
      //cause DS to construct the instance with the updated properties.
      auto instanceI = GetInstance<test::CAInterface>();
      ASSERT_TRUE(instanceI) << "GetService failed for CAInterface";

      //Confirm component instance was created with the correct properties
      instanceProps = instanceI->GetProperties();
      auto uniquePropI = instanceProps.find(uniqueProp[i]);

      EXPECT_TRUE(uniquePropI != instanceProps.end())
        << uniqueProp[i] << " not found in constructed instance";  //not found
      EXPECT_EQ(uniquePropI->second, instance[i]);

    ++i;
  }
}

TEST_F(tServiceComponent, testOptionalConfigPolicyImmediateFalse) // DS_CAI_FTC_5
{

  // Start the test bundle containing the component name.
  std::string componentName           = "sample::ServiceComponentCA1";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA1");

  // Use DS runtime service to validate the component description and
  // Verify that DS is finished creating the component data structures.
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
    GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
    << "Component state should be SATISFIED";

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
  std::string uniqueProp[3] = { "uniqueProp1", "uniqueProp2", "uniqueProp3" };
  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  const std::string instance[3] = { "instance1", "instance2", "instance3" };
  cppmicroservices::AnyMap instanceProps(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  // Repeat a few time to check the expected behavior
  int i = 0;
  while (i < 3) {
    // The Update properties sends an asynchronous request to DS to
    // update the properties. Must wait until DS is finished before we can continue.
    // Use DS runtime service to validate the component properties. May take more
    // than one try.

    props[uniqueProp[i]] = instance[i];
    configuration->Update(props);
    auto result = RepeatTaskUntilOrTimeout(
      [&compDescDTO, &compConfigs, this, &testBundle, &configInstance]() {
        compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(
          testBundle, configInstance);
        if (compDescDTO.name != "") {
          compConfigs =
            this->dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
        }
      },
      [&compConfigs, &instance, &uniqueProp, &i]() -> bool {
        if (compConfigs.size() == 1) {
          auto properties = compConfigs.at(0).properties;
          auto id = properties.find(uniqueProp[i]);
          if (id != properties.end()) {
            return (id->second == instance[i]);
          }
        }
        return false;
      });

    ASSERT_TRUE(result) << "Timed out waiting for Update Configuration"
                           "to complete.";
    EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
    if (i == 0)
    {
      EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
        << "Component instance state should be SATISFIED";
    }
    else
    {
      EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
        << "Component instance state should be ACTIVE";
    }

    //Request a service reference to the new component instance. This will
    //cause DS to construct the instance with the updated properties.
    auto instanceI = GetInstance<test::CAInterface>();
    ASSERT_TRUE(instanceI) << "GetService failed for CAInterface";

    //Confirm component instance was created with the correct properties
    instanceProps = instanceI->GetProperties();
    auto uniquePropI = instanceProps.find(uniqueProp[i]);

    EXPECT_TRUE(uniquePropI != instanceProps.end())
      << uniqueProp[i] << " not found in constructed instance"; //not found
    EXPECT_EQ(uniquePropI->second, instance[i]);

    ++i;
  }
}

TEST_F(tServiceComponent, testRequiredConfigPolicy) // DS_CAI_FTC_6
{
  // Start the test bundle containing the component name.
  std::string componentName           = "sample::ServiceComponentCA20";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA20");

  // Use DS runtime service to validate the component state
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
    GetComponentConfigs(testBundle, componentName, compDescDTO);
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

  // The Update properties sends an asynchronous request to DS to
  // update the properties.
  auto result = RepeatTaskUntilOrTimeout(
    [&compDescDTO, &compConfigs, this, &testBundle, &configInstance]() {
      compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(
        testBundle, configInstance);
      if (compDescDTO.name != "") {
        compConfigs =
          this->dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
      }
    },
    [&compConfigs, &instanceId]() -> bool {
      if (compConfigs.size() == 1) {
        auto properties = compConfigs.at(0).properties;
        auto id = properties.find("uniqueProp");
        if (id != properties.end()) {
          return (id->second == instanceId);
        }
      }
      return false;
    });

  ASSERT_TRUE(result) << "Timed out waiting for Update Configuration"
                         "to complete.";

  // Confirm configuration object presented and component is satisfied.
  compConfigs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
    << "SATISFIED is exepected since configuration object is created.";

  //Request a service reference to the new component instance. This will
  //cause DS to construct the instance with the updated properties.
  auto instance = GetInstance<test::CAInterface>();
  ASSERT_TRUE(instance) << "GetService failed for CAInterface";

  //Confirm component instance was created with the correct properties

  auto instanceProps = instance->GetProperties();
  auto uniqueProp    = instanceProps.find("uniqueProp");

  ASSERT_TRUE(uniqueProp != instanceProps.end())
    << "uniqueProp not found in constructed instance";
  EXPECT_EQ(uniqueProp->second, instanceId);
}

TEST_F(tServiceComponent, testIgnoreConfigPolicy) // DS_CAI_FTC_7
{
  // Start the test bundle containing the component name.
  std::string           componentName = "sample::ServiceComponentCA7";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA7");

  // Use DS runtime service to validate the component state
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
    GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
    << "Ignore policy always allow the component configuration to be satisfied";

  // Get a service reference to ConfigAdmin to create the component instance.
  auto configAdminService =
    GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

  // Create configuration object and try to update property
  auto configuration  = configAdminService->GetConfiguration(componentName);
  auto configInstance = configuration->GetPid();

  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  const std::string instanceId{ "instance1" };
  props["uniqueProp"] = instanceId;
  configuration->Update(props);

  // Try to update properties sends an asynchronous request to DS.
  auto result = RepeatTaskUntilOrTimeout(
    [&compDescDTO, &compConfigs, this, &testBundle, &configInstance]() {
      compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(
        testBundle, configInstance);
      if (compDescDTO.name != "") {
        compConfigs =
          this->dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
      }
    },
    [&compConfigs, &instanceId]() -> bool {
      if (compConfigs.size() == 1) {
        auto properties = compConfigs.at(0).properties;
        auto id = properties.find("uniqueProp");
        if (id != properties.end()) {
          return (id->second == instanceId);
        }
      }
      return false;
    });

  // Property shouldn't be updated since the configuration object won't be used even if present
  ASSERT_FALSE(result) << "Timed out is expected since configuration object is not being used.";
}

} // end of test namespace
