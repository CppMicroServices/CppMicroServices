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
     * Test that the comonent properties have the correct values when
     * the same property is specified in multiple places based on precedence
     */
    TEST_F(tServiceComponent, testPrecedence)
    {
        // Start the test bundle containing the component name.
        std::string componentName = "sample::ServiceComponentCA12";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA12");

        // Use DS runtime service to validate the component state.
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);

        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
            << "Ignore policy component state should be ACTIVE.";

        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

        // Create configuration objects
        auto configuration00 = configAdminService->GetConfiguration(componentName + "_00");
        auto configuration01 = configAdminService->GetConfiguration(componentName + "_01");

        // GetService to make component active
        auto instance = GetInstance<test::CAInterface>();
        ASSERT_TRUE(instance) << "GetSerivce failed for CAInterface";

        cppmicroservices::AnyMap props00(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        props00["GreetingProp"] = cppmicroservices::Any(std::string("hola"));
        auto fut = configuration00->Update(props00);
        fut.get();
        cppmicroservices::AnyMap props01(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        props01["GreetingProp"] = cppmicroservices::Any(std::string("bonjour"));
        fut = configuration01->Update(props01);
        fut.get();
        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        ASSERT_EQ(cppmicroservices::any_cast<std::string>(compConfigs[0].properties.find("GreetingProp")->second),
                  "bonjour");
    }

    /**
     * Test that the comonent properties have the correct values when
     * the same property is specified in multiple places based on precedence
     */
    TEST_F(tServiceComponent, testMultipleConfig)
    {
        // Start the test bundle containing the component name.
        std::string componentName = "sample::ServiceComponentCA12";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA12");

        // Use DS runtime service to validate the component state.
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);

        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
            << "Ignore policy component state should be ACTIVE.";

        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

        // Create configuration objects
        auto configuration00 = configAdminService->GetConfiguration(componentName + "_00");
        auto configuration01 = configAdminService->GetConfiguration(componentName + "_01");

        // GetService to make component active
        auto instance = GetInstance<test::CAInterface>();
        ASSERT_TRUE(instance) << "GetSerivce failed for CAInterface";

        cppmicroservices::AnyMap props00(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        props00["GreetingProp1"] = cppmicroservices::Any(std::string("hola"));
        auto fut = configuration00->Update(props00);
        fut.get();
        cppmicroservices::AnyMap props01(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        props01["GreetingProp2"] = cppmicroservices::Any(std::string("bonjour"));
        fut = configuration01->Update(props01);
        fut.get();
        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        ASSERT_EQ(cppmicroservices::any_cast<std::string>(compConfigs[0].properties.find("GreetingProp")->second),
                  "hello");
        ASSERT_EQ(cppmicroservices::any_cast<std::string>(compConfigs[0].properties.find("GreetingProp1")->second),
                  "hola");
        ASSERT_EQ(cppmicroservices::any_cast<std::string>(compConfigs[0].properties.find("GreetingProp2")->second),
                  "bonjour");
    }

} // namespace test
