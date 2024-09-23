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
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/ServiceObjects.h"

#include "TestInterfaces/Interfaces.hpp"

namespace test
{

    auto const TIMEOUT = std::chrono::milliseconds(2000);

    /*
     * Tests that if a configuration object is created programmatically before
     * the service that is dependent on the configuration object is installed
     * and started, the service is resolved as soon as it is started.
     */
    TEST_F(tServiceComponent, testUpdateConfigBeforeStartingBundleResolvesService)
    {
        cppmicroservices::BundleContext ctx = framework.GetBundleContext();

        // Get a service reference to ConfigAdmin to create the configuration object.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin.";

        // Create configuration object and update properties BEFORE installing and
        // starting the bundle which defines the service.
        auto configuration = configAdminService->GetConfiguration("sample::ServiceComponentCA02");
        configuration->UpdateIfDifferent(std::unordered_map<std::string, cppmicroservices::Any> {
            { "foo", true }
        });

        // Install and start the bundle which has the service
        auto testBundle = ::test::InstallAndStartBundle(ctx, "TestBundleDSCA02");
        ASSERT_TRUE(testBundle);
        std::string const componentName { "sample::ServiceComponentCA02" };

        // Confirm configuration object is available and service is resolved.
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
            << "SATISFIED is exepected since configuration object is created.";

        // Get an instance of the service
        auto instance = GetInstance<test::CAInterface>();
        ASSERT_TRUE(instance) << "GetService failed for CAInterface";

        // Confirm component state is active.
        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        ASSERT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE) << "Component state should be ACTIVE";
    }

    TEST_F(tServiceComponent, testConcurrentUpdateConfigAndActivateService)
    {
        cppmicroservices::BundleContext ctx = framework.GetBundleContext();

        // Get a service reference to ConfigAdmin to create the configuration object.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin.";

        std::promise<void> go;
        std::shared_future<void> ready(go.get_future());
        std::vector<std::promise<void>> readies(2);

        auto updateConfigFuture
            = std::async(std::launch::async,
                         [&ready, &readies, &configAdminService]()
                         {
                             readies[0].set_value();
                             ready.wait();
                             auto configuration = configAdminService->GetConfiguration("sample::ServiceComponentCA02");
                             configuration->UpdateIfDifferent(std::unordered_map<std::string, cppmicroservices::Any> {
                                 { "foo", true }
                             });
                         });

        auto activateServiceFuture = std::async(std::launch::async,
                                                [this, &ready, &readies, &ctx]()
                                                {
                                                    readies[1].set_value();
                                                    ready.wait();
                                                    auto testBundle
                                                        = ::test::InstallAndStartBundle(ctx, "TestBundleDSCA02");
                                                    ASSERT_TRUE(testBundle);
                                                    auto instance = GetInstance<test::CAInterface>();
                                                });

        readies[0].get_future().wait();
        readies[1].get_future().wait();
        go.set_value();
        ASSERT_NO_THROW(updateConfigFuture.get());
        ASSERT_NO_THROW(activateServiceFuture.get());
    }

    /**
     * Tests that a component configuration which requires a configuration object
     * is not satisfied by a default constructed, empty configuration object.
     */
    TEST_F(tServiceComponent, testActivateServiceWithEmptyConfigProps)
    {
        cppmicroservices::BundleContext ctx = framework.GetBundleContext();

        std::string const svcComponentName { "sample::ServiceComponentCA02" };

        // Get a service reference to ConfigAdmin to create the configuration object.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin.";

        auto configuration = configAdminService->GetConfiguration("sample::ServiceComponentCA02");

        auto testBundle = ::test::InstallAndStartBundle(ctx, "TestBundleDSCA02");
        ASSERT_TRUE(testBundle);

        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, svcComponentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE)
            << "UNSATISFIED_REFERENCE is expected because configuration object is not "
               "created.";

        auto instance = GetInstance<test::CAInterface>();
        ASSERT_FALSE(instance) << "GetService returned a valid service with an empty "
                                  "config for CAInterface";

        configuration
            ->UpdateIfDifferent(std::unordered_map<std::string, cppmicroservices::Any> {
                { "foo", true }
        })
            .second.get();

        compConfigs = GetComponentConfigs(testBundle, svcComponentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
            << "SATISFIED is exepected since configuration object is created.";

        instance = GetInstance<test::CAInterface>();
        ASSERT_TRUE(instance) << "GetService failed to return a service for CAInterface";

        compConfigs = GetComponentConfigs(testBundle, svcComponentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
            << "SATISFIED is exepected since configuration object is created.";
    }

    /*
     * Tests that if a configuration object is defined in the manifest.json file
     * the service that is dependent on the configuration object is resolved as soon
     * as it is started.
     */
    TEST_F(tServiceComponent, testConfigObjectInManifestResolvesService)
    {
        std::string const configObject = "mw.dependency";

        // Install and start the bundle containing the configuration object and the
        // service which is dependent on the configuration object.
        std::string componentName = "sample::ServiceComponentCA27";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA27");
        ASSERT_TRUE(testBundle);

        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin.";

        // Confirm that the configuration object has been added to the Configuration
        // Admin repository.
        auto startTime = std::chrono::steady_clock::now();
        bool result = false;
        while (!result
               && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime)
                      <= TIMEOUT)
        {

            auto configObjects = configAdminService->ListConfigurations("(pid=" + configObject + ")");
            if (configObjects.size() >= 1)
            {
                result = true;
            }
        }
        ASSERT_TRUE(result);

        // The state of the component might start in the UNSATISFIED_REFERENCE state but should
        // quickly end up in the SATISFIED state once ConfigurationAdmin notifies DS of the configuration
        // object. DS then changes the state to SATISFIED.
        scr::dto::ComponentDescriptionDTO compDescDTO;
        startTime = std::chrono::steady_clock::now();
        result = false;
        while (!result
               && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime)
                      <= TIMEOUT)
        {
            auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
            if (compConfigs.size() == 1)
            {
                result = compConfigs.at(0).state == scr::dto::ComponentState::SATISFIED;
            }
        }
        ASSERT_TRUE(result);

        // GetService to make component active
        std::shared_ptr<test::CAInterface> service;
        startTime = std::chrono::steady_clock::now();
        while (!service
               && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime)
                      <= TIMEOUT)
        {
            service = GetInstance<test::CAInterface>();
        }
        ASSERT_TRUE(service) << "GetService failed for CAInterface";

        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        ASSERT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE) << "Component state should be ACTIVE";

        // Confirm that the properties match the properties provided in the
        // manifest.json file.
        std::string const bar { "bar" };
        auto serviceProps = service->GetProperties();
        auto foo = serviceProps.find("foo");
        ASSERT_TRUE(foo != serviceProps.end()) << "foo not found in constructed instance";
        EXPECT_EQ(foo->second, bar);

        testBundle.Stop();
    }

    /*
     * Verify that Factory Components will not be registered even if an incorrect Configuration matching exactly Factory
     * Configuration PID is initialized.
     */
    TEST_F(tServiceComponent, testErroneousConfigurationInjection)
    {
        std::string const configName = "my.dependency.pid";

        // Install and start the bundle containing the configuration object and the
        // service which is dependent on the configuration object.
        std::string componentName = "sample::ServiceComponentCA28";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA28");
        ASSERT_TRUE(testBundle);

        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin.";

        // GetService to make component active
        auto invalidService = GetInstance<test::CAInterface>();
        ASSERT_FALSE(invalidService) << "Factory component itself should not be registered";

        auto uniqueVal0 = 1;
        auto configuration0 = configAdminService->GetConfiguration(configName);
        cppmicroservices::AnyMap props0 { cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS,
                                          { { "uniqueKey", uniqueVal0 } } };

        auto fut0 = configuration0->Update(props0);
        ASSERT_FALSE(GetInstance<test::CAInterface>())
            << "Factory component should not be registered even with config matching exactly (with no ~)";

        auto uniqueVal1 = 6;
        auto configuration1 = configAdminService->GetConfiguration(configName + "~1");
        cppmicroservices::AnyMap props1 { cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS,
                                          { { "uniqueKey", uniqueVal1 } } };

        auto fut1 = configuration1->Update(props1);
        fut1.wait();
        auto service0 = GetInstance<test::CAInterface>();
        ASSERT_TRUE(service0) << "Factory instance should be created when a '~' configuration is added";
        ASSERT_TRUE(service0->GetProperties().find("uniqueKey")->second == 6);

        testBundle.Stop();
    }
    /*
     * Tests that if a configuration object is defined in the manifest.json file
     * and the same configuration object is also defined programmatically before the service
     * that is dependent on that configuration object is installed and started, that service
     * is resolved as soon as it is started.
     */
    TEST_F(tServiceComponent, testUpdateConfigBeforeStartingBundleAndManifest)
    {
        std::string componentName = "sample::ServiceComponentCA27";
        std::string configObjectName = "mw.dependency";

        // Get a service reference to ConfigAdmin to create the configuration object.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin.";

        // Create configuration object and update properties BEFORE installing and
        // starting the bundle which defines the service.
        auto configuration = configAdminService->GetConfiguration(configObjectName);
        cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        std::string const fooProp { "notbar" };
        props["foo"] = fooProp;
        configuration->UpdateIfDifferent(props);

        // Install and start the bundle containing the configuration object and the
        // service which is dependent on the configuration object.
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA27");
        ASSERT_TRUE(testBundle);

        // The state of the component might start in the UNSATISFIED_REFERENCE state but should
        // quickly end up in the SATISFIED state once ConfigurationAdmin notifies DS of the configuration
        // object. DS then changes the state to SATISFIED.
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto startTime = std::chrono::steady_clock::now();
        auto timeout = std::chrono::milliseconds(2000);
        bool result = false;
        while (!result
               && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime)
                      <= timeout)
        {
            auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
            if (compConfigs.size() == 1)
            {
                result = compConfigs.at(0).state == scr::dto::ComponentState::SATISFIED;
            }
        }
        ASSERT_TRUE(result);

        // GetService to make component active
        std::shared_ptr<test::CAInterface> instance = GetInstance<test::CAInterface>();

        ASSERT_TRUE(instance) << "GetService failed for CAInterface";

        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        ASSERT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE) << "Component state should be ACTIVE";

        testBundle.Stop();
    }
    /**
     * Verify that a service's configuration can be updated after the service is activated
     * without deactivating and reactivating the service.
     */
    TEST_F(tServiceComponent, testUpdateConfig_Modified) // DS_CAI_FTC_1
    {
        // Start bundle
        std::string componentName = "sample::ServiceComponentCA01";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA01");

        // Use DS runtime service to get the component description and to validate the component state.
        // It should be in the SATISFIED state because the configuration policy is optional.
        // and component is delayed
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        ASSERT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
            << "Component state should be SATISFIED";

        // GetService to make component active
        auto service = GetInstance<test::CAInterface>();
        ASSERT_TRUE(service) << "GetService failed for CAInterface";

        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        ASSERT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE) << "Component state should be ACTIVE";

        // Get a service reference to ConfigAdmin.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

        // Create configuration object
        auto configObject = configAdminService->GetConfiguration(componentName);
        auto configObjInstance = configObject->GetPid();

        // Update property
        cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        std::string const instanceId { "instance1" };
        props["uniqueProp"] = instanceId;
        auto fut = configObject->Update(props);
        fut.get();
        // Validate that the correct properties were updated
        auto serviceProps = service->GetProperties();
        auto uniqueProp = serviceProps.find("uniqueProp");

        ASSERT_TRUE(uniqueProp != serviceProps.end()) << "uniqueProp not found in constructed instance";
        EXPECT_EQ(uniqueProp->second, instanceId);

        // Component should still be active as modified method is present.
        // The configuration is updated without deactivating and reactivating the service.
        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        ASSERT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE) << "Component state should be ACTIVE";
    }

    /**
     * Verify that DS component is deactivated and reactivated with the new configuration,
     * when Modified method throws an exception, while updating a service's configuration.
     */
    TEST_F(tServiceComponent, testUpdateConfig_Exception)
    {
        // Start the test bundle
        std::string componentName = "sample::ServiceComponentCA02";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA02");

        // Use DS runtime service to validate the component description and
        // Verify that DS is finished creating the component data structures.
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE)
            << "Component state should be UNSATISIFIED_REFERENCE";

        // Get a service reference to ConfigAdmin to create the component instance.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

        auto configObject = configAdminService->GetConfiguration(componentName);
        auto configObjInstance = configObject->GetPid();
        cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        auto fut = configObject->Update(props);
        fut.get();
        // Request a service reference to the new component instance.
        auto service = GetInstance<test::CAInterface>();
        ASSERT_TRUE(service) << "GetService failed for CAInterface";

        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE) << "Component state should be ACTIVE";

        // Update property
        std::string const instanceId { "instance1" };
        props["uniqueProp"] = instanceId;
        fut = configObject->Update(props);
        fut.get();
        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
            << "Component state should be SATISFIED";

        // This will cause DS to construct the instance with the updated properties.
        auto newInstance = GetInstance<test::CAInterface>();
        ASSERT_TRUE(newInstance) << "GetService failed for CAInterface";

        // Validate that the correct properties were updated
        auto serviceProps = newInstance->GetProperties();
        auto uniqueProp = serviceProps.find("uniqueProp");

        ASSERT_TRUE(uniqueProp != serviceProps.end()) << "uniqueProp not found in constructed instance";
        EXPECT_EQ(uniqueProp->second, instanceId);
    }

    /**
     * Test that all changes to the configuration objects on which the component
     * is dependent result in a deactivation and reactivation of the component.
     */
    TEST_F(tServiceComponent,
           testUpdateConfig_WithoutModifiedMethodImmediate) // DS_CAI_FTC_3
    {
        // Start the test bundle containing the component name.
        std::string componentName = "sample::ServiceComponentCA03";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA03");

        // Use DS runtime service to validate the component state
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE)
            << "The state should be UNSATISFIED_REFERENCE.";

        // Get a service reference to ConfigAdmin to create the component instance.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

        // Create configuration object
        // Verify that all changes to the configuration objects on which the component is dependent
        // result in a deactivation and reactivation of the component
        std::shared_ptr<cppmicroservices::service::cm::Configuration> configuration;
        std::string const uniqueProp[3] = { "uniqueProp1", "uniqueProp2", "uniqueProp3" };
        cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        std::string const instance[3] = { "instance1", "instance2", "instance3" };

        int i = 0;
        while (i < 3)
        {
            // Activation of the component
            configuration = configAdminService->GetConfiguration(componentName);
            props[uniqueProp[i]] = instance[i];
            auto fut = configuration->Update(props);
            fut.get();
            compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
            EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
            EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
                << "Component instance state should be ACTIVE";

            // Request a service reference to the new component instance. This will
            // cause DS to construct the instance with the updated properties.
            auto instanceI = GetInstance<test::CAInterface>();
            ASSERT_TRUE(instanceI) << "GetService failed for CAInterface";

            // Confirm component instance was created with the correct properties
            auto instanceProps = instanceI->GetProperties();
            auto uniquePropI = instanceProps.find(uniqueProp[i]);

            EXPECT_TRUE(uniquePropI != instanceProps.end()) << uniqueProp[i] << " not found in constructed instance";
            EXPECT_EQ(uniquePropI->second, instance[i]);

            // Deactivation of the component
            fut = configuration->Remove();
            fut.get();
            compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
            EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
            EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE)
                << "Factory instance state should be UNSATISFIED_REFERENCE";

            ++i;
        }
    } // end of testUpdateConfigWithoutModifiedMethodImmediate

    /**
     *  Test component instance won't be constructed or activated until requested.
     */
    TEST_F(tServiceComponent,
           testUpdateConfig_WithoutModifiedMethodDelayed) // DS_CAI_FTC_4
    {
        // Start the test bundle containing the component name.
        std::string componentName = "sample::ServiceComponentCA04";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA04");

        // Use DS runtime service to validate the component state
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE)
            << "The state should be UNSATISFIED_REFERENCE.";

        // Get a service reference to ConfigAdmin to create the component instance.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

        // Create configuration object and update property.
        auto configuration = configAdminService->GetConfiguration(componentName);
        auto configInstance = configuration->GetPid();

        cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        std::string const instanceId { "instance1" };
        props["uniqueProp"] = instanceId;
        auto fut = configuration->Update(props);
        fut.get();
        // GetService to make component active.
        auto instance = GetInstance<test::CAInterface>();
        ASSERT_TRUE(instance) << "GetService failed for CAInterface.";

        // Confirm configuration object presented and check component state.
        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
            << "Component instance state should be ACTIVE.";

        // Confirm component instance was created with the correct properties.
        auto instanceProps = instance->GetProperties();
        auto uniqueProp = instanceProps.find("uniqueProp");

        ASSERT_TRUE(uniqueProp != instanceProps.end()) << "uniqueProp not found in constructed instance.";
        EXPECT_EQ(uniqueProp->second, instanceId);

        // Update property
        std::string const instanceId2 { "instance2" };
        props["uniqueProp"] = instanceId2;
        fut = configuration->Update(props);
        fut.get();
        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
            << "Component state should be SATISFIED";

        // This will cause DS to construct the instance with the updated properties.
        auto newInstance = GetInstance<test::CAInterface>();
        ASSERT_TRUE(newInstance) << "GetService failed for CAInterface";

        // Confirm configuration object presented and check component state.
        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
            << "Component instance state should be ACTIVE.";

        // Validate that the correct properties were updated
        auto serviceProps = newInstance->GetProperties();
        auto uniqueProp2 = serviceProps.find("uniqueProp");

        ASSERT_TRUE(uniqueProp2 != serviceProps.end()) << "uniqueProp not found in constructed instance";
        EXPECT_EQ(uniqueProp2->second, instanceId2);

    } // end of testUpdateConfig_WithoutModifiedMethodDelayed

    /**
     * Verify a service specified with scope as PROTOTYPE in component description
     * returns different instances to GetService calls. When the configuration object is
     * updated verify that the properties for all instances are updated.
     */
    TEST_F(tServiceComponent,
           testUpdateConfig_PrototypeScope_Modified) // DS_CAI_FTC_26
    {
        std::string componentName = "sample::ServiceComponentCA26";

        // Start the test bundle containing the component name.
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA26");

        // Use DS runtime service to validate the component state and scope.
        // This is a bundle with immediate = true (not a delayed component)
        // and configuration-policy = "optional" so it should be activated
        // immediately.
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE) << "The state should be ACTIVE.";
        EXPECT_EQ(compDescDTO.scope, "prototype");

        // Create three instances of the service component.
        cppmicroservices::ServiceReference<test::CAInterface> sRef = context.GetServiceReference<test::CAInterface>();
        EXPECT_TRUE(static_cast<bool>(sRef));
        cppmicroservices::ServiceObjects<test::CAInterface> serviceObjects = context.GetServiceObjects(sRef);
        std::set<std::shared_ptr<test::CAInterface>> instanceSet;
        for (size_t i = 0; i < 3; i++)
        {
            instanceSet.emplace(serviceObjects.GetService());
        }
        // Each GetService call should have returned a valid ptr (not nullptr)
        // and each call should have returned a different instance. Verify that instanceSet
        // contains 3 valid instances.
        EXPECT_TRUE(std::none_of(instanceSet.begin(),
                                 instanceSet.end(),
                                 [](std::shared_ptr<test::CAInterface> const& service) { return service == nullptr; }));
        EXPECT_EQ(instanceSet.size(), 3) << "number of service instances returned must be equal to the number of "
                                            "GetService calls";

        // Create configuration object and update property. This will send an
        // Update notification to DS that should result in DS modifying all of the
        // instances. Since this service component has a Modified method, it should
        // remain ACTIVE.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        EXPECT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";
        auto configuration = configAdminService->GetConfiguration(componentName);
        auto configInstance = configuration->GetPid();

        cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        std::string const newProp { "newProperty" };
        props["uniqueProp"] = newProp;
        auto fut = configuration->Update(props);
        fut.get();

        // Confirm component instances were updated with the new property.
        for (std::shared_ptr<test::CAInterface> service : instanceSet)
        {
            auto instanceProps = service->GetProperties();
            auto uniqueProp = instanceProps.find("uniqueProp");
            EXPECT_TRUE(uniqueProp != instanceProps.end()) << "uniqueProp not found in constructed instance.";
            EXPECT_EQ(uniqueProp->second, newProp);
        }
    }

    /**
     * Verify a service specified with scope as PROTOTYPE in component description
     * returns different instances to each GetService call(unlike a component with a
     * singleton scope that will return the same instance for all GetService calls).
     * Verify that when constructed all instances of the component with prototype scope
     * will be constructed with the properties from the configuration object.
     * Since this service component does not have a Modified method, verify that after an
     * update the component is deactivated and reactivated with the correct properties.
     */
    TEST_F(tServiceComponent,
           testUpdateConfig_PrototypeScope_WithoutModified) // DS_CAI_FTC_25
    {
        // Start the test bundle containing the component name.
        std::string componentName = "sample::ServiceComponentCA04";
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA04");

        // Use DS runtime service to validate the component state and scope.
        // since this component has a configuration-policy = "require" and
        // the configuration object is not yet available, the state should
        // be UNSATISFIED_REFERENCE.
        scr::dto::ComponentDescriptionDTO compDescDTO;
        auto compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE)
            << "The state should be UNSATISFIED_REFERENCE.";
        EXPECT_EQ(compDescDTO.scope, "prototype");

        // Get a service reference to ConfigAdmin to create the configuration object.
        auto configAdminService = GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

        // Create configuration object and update property. This will satisfy
        // the component but because this is a delayed component (immediate= false)
        // it will not yet be activated. A GetService call is required in order
        // for DS to activate the component.
        auto configuration = configAdminService->GetConfiguration(componentName);
        auto configInstance = configuration->GetPid();

        cppmicroservices::AnyMap props(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        std::string const initialProp { "initialProp" };
        props["uniqueProp"] = initialProp;
        auto fut = configuration->Update(props);
        fut.get();

        // Call GetService to make the component active. Call it more than once to
        // Get more than one instance.
        cppmicroservices::ServiceReference<test::CAInterface> sRef = context.GetServiceReference<test::CAInterface>();
        EXPECT_TRUE(static_cast<bool>(sRef));
        cppmicroservices::ServiceObjects<test::CAInterface> serviceObjects = context.GetServiceObjects(sRef);
        std::set<std::shared_ptr<test::CAInterface>> instanceSet;
        for (size_t i = 0; i < 3; i++)
        {
            instanceSet.emplace(serviceObjects.GetService());
        }
        // Each GetService call should have returned a valid ptr (not nullptr)
        // and each call should have returned a different instance.
        EXPECT_TRUE(std::none_of(instanceSet.begin(),
                                 instanceSet.end(),
                                 [](std::shared_ptr<test::CAInterface> const& service) { return service == nullptr; }));
        EXPECT_EQ(instanceSet.size(), 3) << "number of service instances returned must be equal to the number of "
                                            "GetService calls";

        // Confirm component state is ACTIVE and all instances have the correct
        // properties.
        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
            << "Component instance state should be ACTIVE.";

        for (std::shared_ptr<test::CAInterface> service : instanceSet)
        {
            auto instanceProps = service->GetProperties();
            auto uniqueProp = instanceProps.find("uniqueProp");
            EXPECT_TRUE(uniqueProp != instanceProps.end()) << "uniqueProp not found in constructed instance.";
            EXPECT_EQ(uniqueProp->second, initialProp);
        }

        // Update the configuration object. Since no Modified method exists and since
        // this is a delayed component(immediate = false). The update call
        // should result in the component begin deactivated. Verify that the
        // state is SATISFIED.
        std::string const updatedProp { "updatedProp" };
        props["uniqueProp"] = updatedProp;
        fut = configuration->Update(props);
        fut.get();
        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
            << "Component state should be SATISFIED";

        instanceSet.clear();
        // Call GetService to make the component active again.
        sRef = context.GetServiceReference<test::CAInterface>();
        EXPECT_TRUE(static_cast<bool>(sRef));
        serviceObjects = context.GetServiceObjects(sRef);
        for (size_t i = 0; i < 3; i++)
        {
            instanceSet.emplace(serviceObjects.GetService());
        }

        // Confirm component state is ACTIVE and all of the instances have the
        // updated property.
        compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
        EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected.";
        EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
            << "Component instance state should be ACTIVE.";

        for (std::shared_ptr<test::CAInterface> service : instanceSet)
        {
            auto instanceProps = service->GetProperties();
            auto uniqueProp = instanceProps.find("uniqueProp");
            EXPECT_TRUE(uniqueProp != instanceProps.end()) << "uniqueProp not found in constructed instance.";
            EXPECT_EQ(uniqueProp->second, updatedProp);
        }
    }
} // namespace test
