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

#include "TestFixture.hpp"
#include "gtest/gtest.h"

#include "TestInterfaces/Interfaces.hpp"

namespace test {
/**
   * Verify that a service's configuration can be updated after the service is activated 
   * without deactivating and reactivating the service.
   */
TEST_F(tServiceComponent, testUpdateConfig_Modified) //DS_CAI_FTC_1
{
  //Start bundle
    std::string componentName = "sample::ServiceComponentCA1";
    cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA1");

  //Use DS runtime service to get the component description
  scr::dto::ComponentDescriptionDTO compDescDTO =
    dsRuntimeService->GetComponentDescriptionDTO(testBundle,componentName);

  //Use DS runtime service to validate the component state.
  //It should be in the SATISFIED state because the configuration policy is optional.
  //and component is delayed
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
      GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  ASSERT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED);

  //GetService to make component active
  auto sRef1 = context.GetServiceReference<test::CAInterface>();
  ASSERT_TRUE(static_cast<bool>(sRef1)) << "Service must be available";
  auto service = context.GetService<test::CAInterface>(sRef1);
  ASSERT_NE(service, nullptr);

  compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  ASSERT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE);

  //Get a service reference to ConfigAdmin to update configuration objects
  auto caRef =
    framework.GetBundleContext()
      .GetServiceReference<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(caRef) << "GetServiceReference failed for ConfigurationAdmin";
  auto configAdminService =
    framework.GetBundleContext()
      .GetService<cppmicroservices::service::cm::ConfigurationAdmin>(caRef);
  ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

  //Create configuration object
  auto configObject =
    configAdminService->GetConfiguration(componentName);
  auto configObjInstance = configObject->GetPid();

  //Update property
  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  const std::string instanceId{ "instance1" };
  props["uniqueProp"] = instanceId;
  configObject->Update(props);

  auto result = RepeatTaskUntilOrTimeout(
    [&compDescDTO, &compConfigs, this, &testBundle, &configObjInstance]() {
      compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(
        testBundle, configObjInstance);
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

  //Validate that the correct properties were updated
  auto serviceProps = service->GetProperties();
  auto uniqueProp = serviceProps.find("uniqueProp");

  ASSERT_TRUE(uniqueProp != serviceProps.end())
    << "uniqueProp not found in constructed instance";
  EXPECT_EQ(uniqueProp->second, instanceId);

  //Component should still be active as modified method is present.
  //The configuration is updated without deactivating and reactivating the service.
  compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  ASSERT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE);
}

TEST_F(tServiceComponent, testUpdateConfig_Exception)
{
  // Start the test bundle containing the factory component name.
  std::string componentName = "sample::ServiceComponentCA02";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA02");

  // Use DS runtime service to validate the component description and
  // Verify that DS is finished creating the component data structures.
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
    GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state,
            scr::dto::ComponentState::UNSATISFIED_REFERENCE)
    << "Component state should be UNSATISIFIED_REFERENCE";

  // Get a service reference to ConfigAdmin to create the component instance.
  auto configAdminService =
    GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

  auto configObject =
    configAdminService->GetConfiguration(componentName);
  auto configObjInstance = configObject->GetPid();

  // wait for the asynchronous task to take effect
  auto result = RepeatTaskUntilOrTimeout(
    [&compDescDTO, &compConfigs, this, &testBundle, &componentName]() {
      compDescDTO =
        dsRuntimeService->GetComponentDescriptionDTO(testBundle, componentName);
      if (compDescDTO.name != "") {
        compConfigs =
          this->dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
      }
    },
    [&compConfigs]() -> bool {
      return compConfigs.at(0).state == scr::dto::ComponentState::SATISFIED;
    });

  ASSERT_TRUE(result)
    << "Timed out waiting for component state to become SATISFIED.";

  //Request a service reference to the new component instance. 
  auto instance = GetInstance<test::CAInterface>();
  ASSERT_TRUE(instance) << "GetService failed for CAInterface";

  compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "Factory instance state should be ACTIVE";

  //Update property
  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  const std::string instanceId{ "instance1" };
  props["uniqueProp"] = instanceId;
  configObject->Update(props);

  result = RepeatTaskUntilOrTimeout(
    [&compDescDTO, &compConfigs, this, &testBundle, &configObjInstance]() {
      compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(
        testBundle, configObjInstance);
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

  compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
    << "Component state should be SATISFIED";

  //This will cause DS to construct the instance with the updated properties.
  auto newInstance = GetInstance<test::CAInterface>();
  ASSERT_TRUE(newInstance) << "GetService failed for CAInterface";

  // Validate that the correct properties were updated
  auto serviceProps = newInstance->GetProperties();
  auto uniqueProp = serviceProps.find("uniqueProp");

  ASSERT_TRUE(uniqueProp != serviceProps.end())
    << "uniqueProp not found in constructed instance";
  EXPECT_EQ(uniqueProp->second, instanceId);
}
}
