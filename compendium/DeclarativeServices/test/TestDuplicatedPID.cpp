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

#include "gtest/gtest.h"
#include "TestFixture.hpp"

#include "TestInterfaces/Interfaces.hpp"

namespace test {

/**
* Negative test for duplicated PID
*/
TEST_F(tServiceComponent, testDuplicatedPID) // DS_CAI_FTC_17
{
  // Start the test bundle containing duplicate PID.
  std::string componentName           = "sample::ServiceComponentCA17";
  cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSCA17");

  // Use DS runtime service to validate the component description
  scr::dto::ComponentDescriptionDTO compDescDTO =
    dsRuntimeService->GetComponentDescriptionDTO(testBundle, componentName);
  EXPECT_EQ(compDescDTO.name, std::string());
  EXPECT_EQ(compDescDTO.implementationClass, std::string());

  // Get a service reference to ConfigAdmin to create the component instance.
  auto configAdminService =
    GetInstance<cppmicroservices::service::cm::ConfigurationAdmin>();
  ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

  // Create configuration object.
  auto configuration = configAdminService->GetConfiguration(componentName);
  auto configInstance = configuration->GetPid();

  // GetService.
  auto instance = GetInstance<test::CAInterface>();
  ASSERT_FALSE(instance) << "GetService should fail for CAInterface.";
}  // end of testDuplicatedPID

} // end of test namespace
