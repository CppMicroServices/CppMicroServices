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
#include <cppmicroservices/ServiceTracker.h>

namespace test {

TEST_F(tServiceComponent, testSetConfig_AnyMap)
{
  
  // Start the test bundle containing the component name.
  std::string componentName = "sample::ServiceComponentCA08";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA08");

  // Use DS runtime service to validate the component description and
  // Verify that DS is finished creating the component data structures.
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
      << "component state should be SATISFIED";

  // Get a service reference to ConfigAdmin to create the component instance.
  auto configAdminService =
    GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
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
  
  // GetService to make component active
  auto instance = GetInstance<test::CAInterface>();
  ASSERT_TRUE(instance) << "GetService failed for CAInterface";
  
  //Confirm factory instance was created with the correct properties
  auto instanceProps = instance->GetProperties();
  auto uniqueProp = instanceProps.find("uniqueProp");
  
  ASSERT_TRUE(uniqueProp != instanceProps.end())
    << "uniqueProp not found in constructed instance";
  EXPECT_EQ(uniqueProp->second, instanceId);
   
}
}

