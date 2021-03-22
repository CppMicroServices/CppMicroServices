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
   * verify that a service's configuration can be updated after the service is activated 
   * without deactivating and reactivating the service.
   */
TEST_F(tServiceComponent, testUpdateConfig_Modified) //DS_CAI_FTC_1
{
  //start bundle
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCAI1");

  // Use DS runtime service to validate the component description
  scr::dto::ComponentDescriptionDTO compDescDTO =
    dsRuntimeService->GetComponentDescriptionDTO(testBundle,
                                                 "sample::TestBundleDSCAI1");
  EXPECT_EQ(compDescDTO.implementationClass, "sample::TestBundleDSCAI1")
    << "Implementation class in the returned component description must be "
       "sample::TestBundleDSCAI1";

  // Use DS runtime service to validate the component state.
  //It should be in the SATISFIED state because the configuration policy is optional.
  //and component is delayed
  auto compConfigs =
    dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED);

  //getservice to make component active
  auto ctxt = framework.GetBundleContext();
  auto sRef1 = ctxt.GetServiceReference<test::Interface4>();
  EXPECT_TRUE(static_cast<bool>(sRef1)) << "Service must be available";
  auto service = ctxt.GetService<test::Interface4>(sRef1);
  ASSERT_NE(service, nullptr);

  compConfigs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE);

  // Get a service reference to ConfigAdmin to create the component instance.
  auto caRef =
    framework.GetBundleContext()
      .GetServiceReference<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(caRef) << "GetServiceReference failed for ConfigurationAdmin";
  auto configAdminService =
    framework.GetBundleContext()
      .GetService<cppmicroservices::service::cm::ConfigurationAdmin>(caRef);
  ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

  //Create configuration object
  auto configObject =
    configAdminService->GetConfiguration("sample::TestBundleDSCAI1");
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

  //Component should still be active as modified method is present.
  //The configuration is updated without deactivating and reactivating the service.
  compConfigs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE);
}

}