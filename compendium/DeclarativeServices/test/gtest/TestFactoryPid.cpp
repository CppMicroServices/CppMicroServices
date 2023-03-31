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

namespace test
{
    /**
     * Verify a component with default name(=implementation class name) is loaded properly
     */

    TEST_F(tServiceComponent, testFactoryPidConstruction)
    {

        // Start the test bundle containing the factory component name.
        std::string factoryComponentName = "sample::ServiceComponentCA20";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA20");

        // Use DS runtime service to validate the component description and
        // Verify that DS is finished creating the component data structures.
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, factoryComponentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE)
            << "factory component state should be UNSATISIFIED_REFERENCE";
        auto factoryProps = compConfigs.at(0).properties;

        auto factoryProp = factoryProps.find("component.factory");
        ASSERT_TRUE(factoryProp != factoryProps.end()) << "factoryProp not found in factory properties";
        const std::string factoryId { "factory id" };
        EXPECT_EQ(factoryProp->second, factoryId);

        // Get a service reference to ConfigAdmin to create the factory component instance.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

        // Create factory configuration object
        auto factoryConfig = configAdminService->CreateFactoryConfiguration(factoryComponentName);
        auto factoryInstance = factoryConfig->GetPid();

        // CreateFactoryConfiguration created the configuration object on
        // which the component is configured but it created it with no
        // properties. Update the properties before instantiating the factory
        // instance.
        cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        const std::string instanceId { "instance1" };
        props["uniqueProp"] = instanceId;
        auto fut = factoryConfig->Update(props);
        fut.get();
        // Confirm the properties have been updated in DS.
        compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, factoryInstance);
        EXPECT_EQ(compDescDTO.implementationClass, factoryComponentName)
            << "Implementation class in the returned component description must be " << factoryComponentName;

        compConfigs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
            << "Factory instance state should be SATISFIED";

        // Request a service reference to the new component instance. This will
        // cause DS to construct the instance with the updated properties.
        auto instance = GetInstance<test::CAInterface>();
        ASSERT_TRUE(instance) << "GetService failed for CAInterface";

        // Confirm factory instance was created with the correct properties

        auto instanceProps = instance->GetProperties();
        auto uniqueProp = instanceProps.find("uniqueProp");

        ASSERT_TRUE(uniqueProp != instanceProps.end()) << "uniqueProp not found in constructed instance";
        EXPECT_EQ(uniqueProp->second, instanceId);
    }
    TEST_F(tServiceComponent, testFactoryPidConstructionNameDifferentThanClass)
    {

        // Start the test bundle containing the factory component.
        std::string factoryComponentName = "ServiceComponentName";
        std::string factoryComponentClass = "sample::ServiceComponentCA21";
        std::string configurationPid = "ServiceComponentPid";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA21");

        // Get a service reference to ConfigAdmin to create the factory component instance.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

        // Create factory configuration object
        auto factoryConfig = configAdminService->CreateFactoryConfiguration(configurationPid);
        auto factoryInstance = factoryConfig->GetPid();

        // CreateFactoryConfiguration created the configuration object on
        // which the component is configured but it created it with no
        // properties. Update the properties before instantiating the factory
        // instance.
        cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        const std::string instanceId { "instance1" };
        props["uniqueProp"] = instanceId;
        auto fut = factoryConfig->Update(props);
        fut.get();

        // Request a service reference to the new component instance. This will
        // cause DS to construct the instance with the updated properties.
        auto instance = GetInstance<test::CAInterface>();
        ASSERT_TRUE(instance) << "GetService failed for CAInterface";
    }

    /* testFactoryConfigBeforeInstall
       This test creates the configuration objects for the factory instances
       before the factory component has been installed and started.
       Once the factory component is installed and started the instances
       associated with the configuration objects should be registered
       automatically by DS.
    */
    TEST_F(tServiceComponent, testFactoryConfigBeforeInstall)
    {
        std::string configurationPid = "ServiceComponentPid";

        // Get a service reference to ConfigAdmin to create the factory component instances.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

        // Create some factory configuration objects.
        constexpr auto count = 5;
        for (int i = 0; i < count; i++)
        {
            // Create the factory configuration object
            auto factoryConfig = configAdminService->CreateFactoryConfiguration(configurationPid);

            // Update the properties for the factory configuration object
            cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
            const std::string instanceId { "instance" + std::to_string(i) };
            props["uniqueProp"] = instanceId;
            auto fut = factoryConfig->Update(props);
            fut.get();
        }

        // Start the test bundle containing the factory component. This will
        // cause DS to register the factory instances.
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA21");

        // Request service references to the new component instances. This will
        // cause DS to construct the factory instances.
        auto instances = GetInstances<test::CAInterface>();
        EXPECT_EQ(instances.size(), count);
    }
    /* test MultipleFactoryConfig. 
     * This test creates 10 factory objects without waiting for the previous one to complete. 
     */
    TEST_F(tServiceComponent, testMultipleFactoryConfig)
    {
        std::string configurationPid = "ServiceComponentPid";

        // Start the test bundle containing the factory component. 
        cppmicroservices::Bundle testBundle = StartTestBundle(
            "TestBundleDSCA21"); 
        
        // Get a service reference to ConfigAdmin to create the factory component instances.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

        // Create some factory configuration objects. Don't wait for one to complete before 
        // creating the next one.
        constexpr auto count = 10;
        std::vector<std::shared_future<void>> futures;

        for (int i = 0; i < count; i++)
        {
            // Create the factory configuration object
            auto factoryConfig = configAdminService->CreateFactoryConfiguration(configurationPid);

            // Update the properties for the factory configuration object
            cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
            const std::string instanceId { "instance" + std::to_string(i) };
            props["uniqueProp"] = instanceId;
            auto fut = factoryConfig->Update(props);
            futures.push_back(fut);
        }

        // Wait for all factory objects to finish updating.
        for (auto const& item : futures) {
            item.get();
         } 

        // Request service references to the new component instances. This will
        // cause DS to construct the factory instances.
        auto instances = GetInstances<test::CAInterface>();
        EXPECT_EQ(instances.size(), count);
    }
} // namespace test
