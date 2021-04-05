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

TEST_F(tServiceComponent, testUpdateConfigWithoutModifiedMethodImmediate) // DS_CAI_FTC_3
{
  // Start the test bundle containing the component name.
  std::string componentName = "sample::ServiceComponentCA03";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA03");

  // Use DS runtime service to validate the component state
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
    GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE)
    << "The state should be UNSATISFIED_REFERENCE.";

  // Get a service reference to ConfigAdmin to create the component instance.
  auto configAdminService =
    GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

  // Create configuration object
  // Verify that all changes to the configuration objects on which the component is dependent
  // result in a deactivation and reactivation of the component
  std::shared_ptr<cppmicroservices::service::cm::Configuration> configuration;
  const std::string uniqueProp[3] = {"uniqueProp1", "uniqueProp2", "uniqueProp3" };
  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  const std::string instance[3] = {"instance1", "instance2", "instance3"};

  int i = 0;
  while (i < 3) {
    // Activation of the component
    configuration        = configAdminService->GetConfiguration(componentName);
    props[uniqueProp[i]] = instance[i];
    configuration->Update(props);

    EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
    EXPECT_EQ(compConfigs.at(0).state,
              scr::dto::ComponentState::UNSATISFIED_REFERENCE)
      << "Component instance state should be UNSATISFIED_REFERENCE";

    // Request a service reference to the new component instance. This will
    // cause DS to construct the instance with the updated properties.
    auto instanceI = GetInstance<test::CAInterface>();
    ASSERT_TRUE(instanceI) << "GetService failed for CAInterface";

    // Confirm component instance was created with the correct properties
    auto instanceProps = instanceI->GetProperties();
    auto uniquePropI   = instanceProps.find(uniqueProp[i]);

    EXPECT_TRUE(uniquePropI != instanceProps.end())
      << uniqueProp[i] << " not found in constructed instance";
    EXPECT_EQ(uniquePropI->second, instance[i]);

    // Deactivation of the component
    configuration->Remove();

    EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
    EXPECT_EQ(compConfigs.at(0).state,
              scr::dto::ComponentState::UNSATISFIED_REFERENCE)
      << "Factory instance state should be UNSATISFIED_REFERENCE";

    ++i;
  }
}

} // end of test namespace
