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
     * Test the component instance is constructed without the configuration object is
     * present and the component should be constructed with no properties.
     */
    TEST_F(tServiceComponent,
           testOptionalConfigPolicyWithoutConfigObj) // DS_CAI_FTC_5
    {
        // Start the test bundle containing the component name.
        std::string componentName = "sample::ServiceComponentCA05";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA05");

        // Use DS runtime service to validate the component state.
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
            << "The state should be active once the component instance is constructed.";

        // GetService to make component active.
        auto instance = GetInstance<test::CAInterface>();
        ASSERT_TRUE(instance) << "GetService failed for CAInterface.";

        // Confirm component instance was created with the properties has default value.
        EXPECT_TRUE(instance->GetProperties().empty()) << "Property instance should only have default value.";
    } // end of testOptionalConfigPolicyWithoutConfigObj

    /**
     * Test the component instance is constructed with the configuration object is present
     * and the component should be constructed with the correct properties.
     */
    TEST_F(tServiceComponent, testOptionalConfigPolicyWithConfigObj) // DS_CAI_FTC_5
    {
        // Start the test bundle containing the component name.
        std::string componentName = "sample::ServiceComponentCA05a";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA05a");

        // Use DS runtime service to validate the component state.
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
            << "Component state should be SATISFIED.";

        // Get a service reference to ConfigAdmin to create the component instance.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin.";

        // Create configuration object and update property.
        auto configuration = configAdminService->GetConfiguration(componentName);
        auto configInstance = configuration->GetPid();

        cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        const std::string instanceId { "instance1" };
        props["uniqueProp"] = instanceId;
        auto fut = configuration->Update(props);
        fut.get();

        // Confirm configuration object presented and check component state.
        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
            << "Component instance state should be SATISFIED.";

        // GetService to make component active.
        auto instance = GetInstance<test::CAInterface>();
        ASSERT_TRUE(instance) << "GetService failed for CAInterface.";

        // Confirm component instance was created with the correct properties.
        auto instanceProps = instance->GetProperties();
        auto uniqueProp = instanceProps.find("uniqueProp");

        ASSERT_TRUE(uniqueProp != instanceProps.end()) << "uniqueProp not found in constructed instance.";
        EXPECT_EQ(uniqueProp->second, instanceId);

    } // end of testOptionalConfigPolicyWithConfigObj

    TEST_F(tServiceComponent, testRequiredConfigPolicy) // DS_CAI_FTC_6
    {
        // Start the test bundle containing the component name.
        std::string componentName = "sample::ServiceComponentCA02";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA02");

        // Use DS runtime service to validate the component state.
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE)
            << "UNSATISFIED_REFERENCE is expected because configuration object is not "
               "created.";

        // Get a service reference to ConfigAdmin to create the component instance.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin.";

        // Create configuration object and update property.
        auto configuration = configAdminService->GetConfiguration(componentName);
        auto configInstance = configuration->GetPid();

        cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        const std::string instanceId { "instance1" };
        props["uniqueProp"] = instanceId;
        auto fut = configuration->Update(props);
        fut.get();
        // Confirm configuration object presented and check component state.
        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
            << "SATISFIED is exepected since configuration object is created.";

        // GetService to make component active.
        auto instance = GetInstance<test::CAInterface>();
        ASSERT_TRUE(instance) << "GetService failed for CAInterface.";

        // Confirm component instance was created with the correct properties.
        auto instanceProps = instance->GetProperties();
        auto uniqueProp = instanceProps.find("uniqueProp");

        ASSERT_TRUE(uniqueProp != instanceProps.end()) << "uniqueProp not found in constructed instance.";
        EXPECT_EQ(uniqueProp->second, instanceId);
    } // end of testRequiredConfigPolicy

    /**
     * Test the component instance is created with default properties without the configuration object.
     */
    TEST_F(tServiceComponent,
           testIgnoreConfigPolicyWithoutConfigObj) // DS_CAI_FTC_7
    {
        // Start the test bundle containing the component name.
        std::string componentName = "sample::ServiceComponentCA07";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA07");

        // Use DS runtime service to validate the component state.
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);

        // Confirm configuration object presented and check component state.
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
            << "Ignore policy component state should be ACTIVE.";

        // GetService to make component active.
        auto instance = GetInstance<test::CAInterface>();
        ASSERT_TRUE(instance) << "GetService failed for CAInterface.";

        // Confirm component instance was created with the properties has default value.
        auto instanceProps = instance->GetProperties();
        EXPECT_TRUE(instanceProps.empty()) << "Property instance should be empty.";
    } // end of testIgnoreConfigPolicyWithoutConfigObj

    /**
     * Test the component instance is created with default properties not from the configuration object.
     */
    TEST_F(tServiceComponent, testIgnoreConfigPolicyWithConfigObj) // DS_CAI_FTC_7
    {
        // Start the test bundle containing the component name.
        std::string componentName = "sample::ServiceComponentCA07";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA07");

        // Use DS runtime service to validate the component state.
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
            << "Ignore policy component state should be ACTIVE.";

        // Get a service reference to ConfigAdmin to create the component instance.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin.";

        // Create configuration object, try to update property
        // and property update should be ignored.
        auto configuration = configAdminService->GetConfiguration(componentName);
        auto configInstance = configuration->GetPid();

        cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        const std::string instanceId { "instance1" };
        props["uniqueProp"] = instanceId;
        auto fut = configuration->Update(props);
        fut.get();
        // Confirm configuration object presented and check component state.
        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
            << "ACTIVE is exepected since configuration object is created.";

        // GetService to make component active.
        auto instance = GetInstance<test::CAInterface>();
        ASSERT_TRUE(instance) << "GetService failed for CAInterface.";

        // Confirm component instance was created with the properties has default value.
        EXPECT_TRUE(instance->GetProperties().empty()) << "Property instance should only have default value.";
    } // end of testIgnoreConfigPolicyWithConfigObj

} // namespace test
