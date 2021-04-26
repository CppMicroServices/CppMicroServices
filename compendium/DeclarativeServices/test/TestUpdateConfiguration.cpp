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

namespace test {
/**
   * Verify that a service's configuration can be updated after the service is activated
   * without deactivating and reactivating the service.
   */
TEST_F(tServiceComponent, testUpdateConfig_Modified) //DS_CAI_FTC_1
{
  // Start bundle
  std::string componentName = "sample::ServiceComponentCA01";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA01");

  // Use DS runtime service to get the component description and to validate the component state.
  // It should be in the SATISFIED state because the configuration policy is optional.
  // and component is delayed
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
    GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  ASSERT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
    << "Component state should be SATISFIED";

  // GetService to make component active
  auto service = GetInstance<test::CAInterface>();
  ASSERT_TRUE(service) << "GetService failed for CAInterface";

  compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  ASSERT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "Component state should be ACTIVE";

  // Get a service reference to ConfigAdmin.
  auto configAdminService =
    GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

  // Create configuration object
  auto configObject = configAdminService->GetConfiguration(componentName);
  auto configObjInstance = configObject->GetPid();

  // Update property
  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  const std::string instanceId{ "instance1" };
  props["uniqueProp"] = instanceId;
  configObject->Update(props);

  // Validate that the correct properties were updated
  auto serviceProps = service->GetProperties();
  auto uniqueProp = serviceProps.find("uniqueProp");

  ASSERT_TRUE(uniqueProp != serviceProps.end())
    << "uniqueProp not found in constructed instance";
  EXPECT_EQ(uniqueProp->second, instanceId);

  // Component should still be active as modified method is present.
  // The configuration is updated without deactivating and reactivating the service.
  compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  ASSERT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "Component state should be ACTIVE";
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

  auto configObject = configAdminService->GetConfiguration(componentName);
  auto configObjInstance = configObject->GetPid();
  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  configObject->Update(props);

  // Request a service reference to the new component instance.
  auto service = GetInstance<test::CAInterface>();
  ASSERT_TRUE(service) << "GetService failed for CAInterface";

  compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "Component state should be ACTIVE";

  // Update property
  const std::string instanceId{ "instance1" };
  props["uniqueProp"] = instanceId;
  configObject->Update(props);

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

  ASSERT_TRUE(uniqueProp != serviceProps.end())
    << "uniqueProp not found in constructed instance";
  EXPECT_EQ(uniqueProp->second, instanceId);
}

/**
  * Test that all changes to the configuration objects on which the component 
  * is dependent result in a deactivation and reactivation of the component.
  */
TEST_F(tServiceComponent, testUpdateConfig_WithoutModifiedMethodImmediate) // DS_CAI_FTC_3
{
  // Start the test bundle containing the component name.
  std::string componentName = "sample::ServiceComponentCA03";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA03");

  // Use DS runtime service to validate the component state
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
    GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state,
            scr::dto::ComponentState::UNSATISFIED_REFERENCE)
    << "The state should be UNSATISFIED_REFERENCE.";

  // Get a service reference to ConfigAdmin to create the component instance.
  auto configAdminService =
    GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

  // Create configuration object
  // Verify that all changes to the configuration objects on which the component is dependent
  // result in a deactivation and reactivation of the component
  std::shared_ptr<cppmicroservices::service::cm::Configuration> configuration;
  const std::string uniqueProp[3] = { "uniqueProp1",
                                      "uniqueProp2",
                                      "uniqueProp3" };
  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  const std::string instance[3] = { "instance1", "instance2", "instance3" };

  int i = 0;
  while (i < 3) {
    // Activation of the component
    configuration = configAdminService->GetConfiguration(componentName);
    props[uniqueProp[i]] = instance[i];
    configuration->Update(props);

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

    EXPECT_TRUE(uniquePropI != instanceProps.end())
      << uniqueProp[i] << " not found in constructed instance";
    EXPECT_EQ(uniquePropI->second, instance[i]);

    // Deactivation of the component
    configuration->Remove();

    compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
    EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
    EXPECT_EQ(compConfigs.at(0).state,
              scr::dto::ComponentState::UNSATISFIED_REFERENCE)
      << "Factory instance state should be UNSATISFIED_REFERENCE";

    ++i;
  }
} // end of testUpdateConfigWithoutModifiedMethodImmediate

/**
  *  Test component instance won't be constructed or activated until requested.
  */
TEST_F(tServiceComponent, testUpdateConfig_WithoutModifiedMethodDelayed) // DS_CAI_FTC_4
{
  // Start the test bundle containing the component name.
  std::string componentName = "sample::ServiceComponentCA04";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA04");

  // Use DS runtime service to validate the component state
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
    GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state,
            scr::dto::ComponentState::UNSATISFIED_REFERENCE)
    << "The state should be UNSATISFIED_REFERENCE.";

  // Get a service reference to ConfigAdmin to create the component instance.
  auto configAdminService =
    GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

  // Create configuration object and update property.
  auto configuration = configAdminService->GetConfiguration(componentName);
  auto configInstance = configuration->GetPid();

  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  const std::string instanceId{ "instance1" };
  props["uniqueProp"] = instanceId;
  configuration->Update(props);

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

  ASSERT_TRUE(uniqueProp != instanceProps.end())
    << "uniqueProp not found in constructed instance.";
  EXPECT_EQ(uniqueProp->second, instanceId);

} // end of testUpdateConfig_WithoutModifiedMethodDelayed

/**
  * Verify a service specified with scope as BUNDLE in component description
  * is published with the correct scope and calls to GetService from the same
  * bundle context return the same instance but calls from different contexts
  * result in different instances of the service
  */

TEST_F(tServiceComponent, testUpdateConfig_BundleScope_Modified) //DS_CAI_FTC_26
{
  // Start the test bundle containing the component name.
  std::string componentName = "sample::ServiceComponentCA26";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA26");

  // Use DS runtime service to validate the component state
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
    GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.at(0).description.scope,
            cppmicroservices::Constants::SCOPE_BUNDLE);
  EXPECT_EQ(compConfigs.at(0).state,
            scr::dto::ComponentState::UNSATISFIED_REFERENCE)
    << "The state should be UNSATISFIED_REFERENCE.";

  // Get a service reference to ConfigAdmin to create the component instance.
  auto configAdminService =
    GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

  // Create configuration object and update property.
  auto configuration = configAdminService->GetConfiguration(componentName);
  auto configInstance = configuration->GetPid();

  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  const std::string instanceId{ "instance1" };
  props["uniqueProp"] = instanceId;
  configuration->Update(props);

  // Use DS runtime service to validate the component state
  compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
    << "The state should be SATISFIED.";

  // Check Service scope
  cppmicroservices::ServiceReference<test::CAInterface> sRef =
    context.GetServiceReference<test::CAInterface>();
  EXPECT_TRUE(static_cast<bool>(sRef));
  auto serviceScope =
    sRef.GetProperty(cppmicroservices::Constants::SERVICE_SCOPE);
  EXPECT_EQ(compConfigs.at(0).description.scope,
            serviceScope.ToStringNoExcept());

  // Get Service Objects
  cppmicroservices::ServiceObjects<test::CAInterface> serviceObjects =
    context.GetServiceObjects(sRef);
  std::set<std::shared_ptr<test::CAInterface>> instanceSet;
  for (size_t i = 0; i < 5; i++) {
    instanceSet.emplace(serviceObjects.GetService());
  }

  // Use DS runtime service to validate the component state
  compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "The state should be ACTIVE.";

  // Start helper testBundle
  std::string helperComponentName = "sample::ServiceComponentCA01";
  cppmicroservices::Bundle helperTestBundle =
    StartTestBundle("TestBundleDSCA01");

  compConfigs =
    GetComponentConfigs(helperTestBundle, helperComponentName, compDescDTO);
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
    << "The state should be SATISFIED.";

  // Get Service Objects
  auto helperBundleCtxt = helperTestBundle.GetBundleContext();
  auto sRef1 = helperBundleCtxt.GetServiceReference<test::CAInterface>();
  cppmicroservices::ServiceObjects<test::CAInterface> serviceObjects1 =
    helperBundleCtxt.GetServiceObjects(sRef1);
  for (size_t i = 0; i < 5; i++) {
    instanceSet.emplace(serviceObjects1.GetService());
  }

  EXPECT_TRUE(
    std::none_of(instanceSet.begin(),
                 instanceSet.end(),
                 [](const std::shared_ptr<test::CAInterface>& service) {
                   return service == nullptr;
                 }));
  EXPECT_EQ(instanceSet.size(), 2)
    << "number of service instances returned must be equal to the number of "
       "distinct contexts used to call GetService";

  for (std::shared_ptr<test::CAInterface> service : instanceSet) {
    // Confirm component instances were created with the correct properties.
    auto instanceProps = service->GetProperties();
    auto uniqueProp = instanceProps.find("uniqueProp");

    ASSERT_TRUE(uniqueProp != instanceProps.end())
      << "uniqueProp not found in constructed instance.";
    EXPECT_EQ(uniqueProp->second, instanceId);
  }

  const std::string newInstanceId{ "newInstance" };
  props["uniqueProp"] = newInstanceId;
  configuration->Update(props);

  for (std::shared_ptr<test::CAInterface> service : instanceSet) {
    // Confirm component instances were updated with the correct properties.
    auto instanceProps = service->GetProperties();
    auto uniqueProp = instanceProps.find("uniqueProp");

    ASSERT_TRUE(uniqueProp != instanceProps.end())
      << "uniqueProp not found in constructed instance.";
    EXPECT_EQ(uniqueProp->second, newInstanceId);
  }

  // Use DS runtime service to validate the component state
  compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "The state should be ACTIVE.";

  instanceSet.clear();
}

TEST_F(tServiceComponent, testUpdateConfig_PrototypeScope_Modified) //DS_CAI_FTC_24
{
  // Start the test bundle containing the component name.
  std::string componentName = "sample::ServiceComponentCA24";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA24");

  // Use DS runtime service to validate the component state
  scr::dto::ComponentDescriptionDTO compDescDTO;
  auto compConfigs =
    GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.at(0).description.scope,
            cppmicroservices::Constants::SCOPE_PROTOTYPE);
  EXPECT_EQ(compConfigs.at(0).state,
            scr::dto::ComponentState::UNSATISFIED_REFERENCE)
    << "The state should be UNSATISFIED_REFERENCE.";

  // Get a service reference to ConfigAdmin to create the component instance.
  auto configAdminService =
    GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

  // Create configuration object and update property.
  auto configuration = configAdminService->GetConfiguration(componentName);
  auto configInstance = configuration->GetPid();

  cppmicroservices::AnyMap props(
    cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  const std::string instanceId{ "instance1" };
  props["uniqueProp"] = instanceId;
  configuration->Update(props);

  // Use DS runtime service to validate the component state
  compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED)
    << "The state should be SATISFIED.";

  // Check Service scope
  cppmicroservices::ServiceReference<test::CAInterface> sRef =
    context.GetServiceReference<test::CAInterface>();
  EXPECT_TRUE(static_cast<bool>(sRef));
  auto serviceScope =
    sRef.GetProperty(cppmicroservices::Constants::SERVICE_SCOPE);
  EXPECT_EQ(compConfigs.at(0).description.scope,
            serviceScope.ToStringNoExcept());

  // Get Service Objects
  cppmicroservices::ServiceObjects<test::CAInterface> serviceObjects =
    context.GetServiceObjects(sRef);
  std::set<std::shared_ptr<test::CAInterface>> instanceSet;
  for (size_t i = 0; i < 5; i++) {
    instanceSet.emplace(serviceObjects.GetService());
  }

  // Use DS runtime service to validate the component state
  compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "The state should be ACTIVE.";

  EXPECT_TRUE(
    std::none_of(instanceSet.begin(),
                 instanceSet.end(),
                 [](const std::shared_ptr<test::CAInterface>& service) {
                   return service == nullptr;
                 }));
  EXPECT_EQ(instanceSet.size(), 5)
    << "number of service instances returned must be equal to the number of "
       "GetService calls";

  // Confirm component instances were created with the same properties.
  for (std::shared_ptr<test::CAInterface> service : instanceSet) {
    auto instanceProps = service->GetProperties();
    auto uniqueProp = instanceProps.find("uniqueProp");

    ASSERT_TRUE(uniqueProp != instanceProps.end())
      << "uniqueProp not found in constructed instance.";
    EXPECT_EQ(uniqueProp->second, instanceId);
  }

  const std::string newInstanceId{ "newInstance" };
  props["uniqueProp"] = newInstanceId;
  configuration->Update(props);

  // Confirm component instances were updated with the correct properties.
  for (std::shared_ptr<test::CAInterface> service : instanceSet) {
    auto instanceProps = service->GetProperties();
    auto uniqueProp = instanceProps.find("uniqueProp");

    ASSERT_TRUE(uniqueProp != instanceProps.end())
      << "uniqueProp not found in constructed instance.";
    EXPECT_EQ(uniqueProp->second, newInstanceId);
  }

  // Use DS runtime service to validate the component state
  compConfigs = GetComponentConfigs(testBundle, componentName, compDescDTO);
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE)
    << "The state should be ACTIVE.";

  instanceSet.clear();
}

}
