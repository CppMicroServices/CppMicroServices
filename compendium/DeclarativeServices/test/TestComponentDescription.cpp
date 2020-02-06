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

namespace test
{
  /**
   * Verify a component with default name(=implementation class name) is loaded properly
   */
  TEST_F(tServiceComponent, testComponentLoad_DefaultComponentName) //DS_TOI_2
  {
    cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSTOI2");
    scr::dto::ComponentDescriptionDTO compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent2");
    EXPECT_EQ(compDescDTO.name, compDescDTO.implementationClass) << "component name and implementation class must be different";
    EXPECT_EQ(compDescDTO.implementationClass, "sample::ServiceComponent2") << "Implementation class in the returned component description must be sample::ServiceComponent2";
    dsRuntimeService->EnableComponent(compDescDTO);
    EXPECT_EQ(dsRuntimeService->IsComponentEnabled(compDescDTO), true) << "current state reported by the runtime service must match the initial state in component description";
    auto bc = framework.GetBundleContext();
    auto sRef = bc.GetServiceReference<test::Interface1>();
    EXPECT_TRUE(static_cast<bool>(sRef));
    auto service = bc.GetService(sRef);
    EXPECT_NE(service, nullptr);
    testBundle.Stop();
  }

  /**
   * Verify a component with custom name is loaded properly
   */
  TEST_F(tServiceComponent, testComponentLoad_CustomComponentName) //DS_TOI_3
  {
    cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSTOI3");
    scr::dto::ComponentDescriptionDTO compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sampleServiceComponent");
    EXPECT_NE(compDescDTO.name, compDescDTO.implementationClass) << "component name and implementation class must be different";
    EXPECT_EQ(compDescDTO.name, "sampleServiceComponent") << "Name in the returned component description must be sampleServiceComponent";
    EXPECT_EQ(compDescDTO.implementationClass, "sample::ServiceComponent3") << "Implementation class in the returned component description must be sample::ServiceComponent3";
    EXPECT_EQ(dsRuntimeService->IsComponentEnabled(compDescDTO), true) << "current state reported by the runtime service must match the initial state in component description";
    auto bc = framework.GetBundleContext();
    auto sRef = bc.GetServiceReference<test::Interface1>();
    EXPECT_TRUE(static_cast<bool>(sRef));
    auto service = bc.GetService(sRef);
    EXPECT_NE(service, nullptr);
    testBundle.Stop();
  }
}
