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
   * Verify a component with default name(=implementation class name) is loaded properly
   */
TEST_F(tServiceComponent, testFactoryPidConstruction) 
{ 
     
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSTOI20");
  
  // Use DS runtime service to validate the component description
  scr::dto::ComponentDescriptionDTO compDescDTO =
    dsRuntimeService->GetComponentDescriptionDTO(testBundle,
                                                "sample::ServiceComponent20");
  EXPECT_EQ(compDescDTO.implementationClass, "sample::ServiceComponent20")
    << "Implementation class in the returned component description must be "
       "sample::ServiceComponent20";

  // Use DS runtime service to validate the component state. It should be in
  // the UNSATISFIED_REFERENCE state because the component is dependent on 
  // a configuration object that has not been created yet. 
  auto compConfigs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE);

  //Create factory instance, update configuration object 
  auto factoryConfig = configAdminService->CreateFactoryConfiguration(
    "sample::ServiceComponent20");
  auto factoryInstance = factoryConfig->GetPid();

  // CreateFactoryConfiguration will send an asynchronous request to DS to 
  // create the component data structures. Must wait until DS is finished before we can 
  // continue. We will know DS is finished when GetComponentDescriptionDTO 
  // returns the correct implementationClass. 
  // ?? Is this the best way to do this? TestConfigAdmin.cpp has some complicated
  // polling functions. Do we need that?
  std::string implementationClass;
  int count = 0;
  while  (implementationClass.empty() && count < 5) {
    compDescDTO =
    dsRuntimeService->GetComponentDescriptionDTO(testBundle, factoryInstance);
    implementationClass = compDescDTO.implementationClass;
    if (implementationClass.empty()){
      std::this_thread::sleep_for(DEFAULT_POLL_PERIOD);
    } 
    count++;
  }

  // Use DS runtime service to validate the component description. 
    EXPECT_EQ(compDescDTO.implementationClass, "sample::ServiceComponent20")
    << "Implementation class in the returned component description must be "
       "sample::ServiceComponent20";
  EXPECT_EQ(compDescDTO.name, factoryInstance)
    << "Name in the returned component description must be " << factoryInstance;

  // Use DS runtime service to validate the component state
  // CreateFactoryConfiguration created the configuration object on
  // which the component is dependent. The component is now SATISFIED and
  // has been registered. It has not yet been constructed because the
  // manifest.json file contains the immediate=false attribute.
  compConfigs =
    dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state,
            scr::dto::ComponentState::SATISFIED);

  // CreateFactoryConfiguration created the configuration object on
  // which the component is configured but it created it with no 
  // properties. Update the properties before instantiating the component.
  cppmicroservices::AnyMap props(
  cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  const std::string instanceId{ "instance1" };
  props["uniqueProp"] = instanceId;
  factoryConfig->Update(props);
  testBundle.Stop();
 
}

}