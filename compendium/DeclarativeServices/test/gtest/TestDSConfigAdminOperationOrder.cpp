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
     * Verify that DS must complete modification before another modification can be applied.
     */
    TEST_F(tServiceComponent, testModificationOrder) // DS_CAI_FTC_22
    {
        // Start the test bundle containing the component name.
        std::string componentName = "sample::ServiceComponentCA24";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA24");

        // Use DS runtime service to validate the component description and
        // Verify that DS is finished creating the component data structures.
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE)
            << "component state should be UNSATISFIED_REFERENCE";

        // Get a service reference to ConfigAdmin to create the component instance.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

        // Create configuration object
        auto configObject = configAdminService->GetConfiguration(componentName);

        // Update property
        cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        const std::string instanceId { "instance1" };
        props["uniqueProp"] = instanceId;
        auto fut = configObject->Update(props);
        fut.get();

        // GetService to make component active
        auto service = GetInstance<test::CAInterface>();
        ASSERT_TRUE(service) << "GetService failed for CAInterface";

        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        ASSERT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE) << "Component state should be ACTIVE";

        // Update property
        const std::string update1 { "update1" };
        props["uniqueProp"] = update1;
        configObject->Update(props);

        const std::string update2 { "update2" };
        props["uniqueProp"] = update2;
        fut = configObject->Update(props);
        fut.get();

        // Validate that the second update happened after the first update
        auto serviceProps = service->GetProperties();
        auto uniqueProp = serviceProps.find("uniqueProp");

        ASSERT_TRUE(uniqueProp != serviceProps.end()) << "uniqueProp not found in constructed instance";
        EXPECT_EQ(uniqueProp->second, update2);
    }
} // namespace test
