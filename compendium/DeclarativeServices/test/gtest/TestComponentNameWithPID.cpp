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

namespace test
{

    /**
     * Test using custom component name
     */
    TEST_F(tServiceComponent, testComponentNameWithPID) // DS_CAI_FTC_16
    {
        // Start the test bundle containing the custom component name.
        std::string componentName = "sampleServiceComponentCA16";
        std::string implementationClass = "sample::ServiceComponentCA16";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA16");

        // Use DS runtime service to validate the component state
        scr::dto::ComponentDescriptionDTO compDescDTO
            = dsRuntimeService->GetComponentDescriptionDTO(testBundle, componentName);
        EXPECT_EQ(compDescDTO.name, componentName) << "Component name must be " << componentName;
        EXPECT_EQ(compDescDTO.implementationClass, implementationClass)
            << "Implementation class must be " << implementationClass;

        auto compConfigs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE)
            << "The state should be UNSATISFIED_REFERENCE.";

        // Get a service reference to ConfigAdmin to create the component instance.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

        // Create configuration object and update property.
        auto configuration = configAdminService->GetConfiguration(implementationClass);
        auto configInstance = configuration->GetPid();

        cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        const std::string instanceId { "instance1" };
        props["uniqueProp"] = instanceId;
        auto fut = configuration->Update(props);
        fut.get();
        // GetService to make component active.
        auto instance = GetInstance<test::CAInterface>();
        ASSERT_TRUE(instance) << "GetService failed for CAInterface.";

        // Confirm configuration object presented and check component state.
        compConfigs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
            << "Component instance state should be ACTIVE.";

        // Confirm component instance was created with the correct properties.
        auto instanceProps = instance->GetProperties();
        auto uniqueProp = instanceProps.find("uniqueProp");

        ASSERT_TRUE(uniqueProp != instanceProps.end()) << "uniqueProp not found in constructed instance.";
        EXPECT_EQ(uniqueProp->second, instanceId);
    } // end of testComponentNameWithPID

} // namespace test
