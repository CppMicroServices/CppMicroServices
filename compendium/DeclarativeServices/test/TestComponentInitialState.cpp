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
   * Verify state changes for a component with initial state as ENABLED
   */
  TEST_F(tServiceComponent, testInitialEnabled) //testDS_TOI_1
  {
    std::string testBundleName("TestBundleDSTOI1");
    cppmicroservices::Bundle testBundle = StartTestBundle(testBundleName);
    scr::dto::ComponentDescriptionDTO compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent");
    EXPECT_EQ(compDescDTO.name, "sample::ServiceComponent") << "Name in the returned component description must be sample::ServiceComponentImpl";
    EXPECT_EQ(compDescDTO.defaultEnabled, true) << "Component's initial state in component description must be ENABLED";
    EXPECT_EQ(dsRuntimeService->IsComponentEnabled(compDescDTO), true) << "current state reported by the runtime service must match the initial state in component description";
    auto sRef = framework.GetBundleContext().GetServiceReference<test::Interface1>();
    EXPECT_TRUE(static_cast<bool>(sRef));
    auto fut = dsRuntimeService->DisableComponent(compDescDTO);
    EXPECT_NO_THROW(fut.get());
    EXPECT_EQ(dsRuntimeService->IsComponentEnabled(compDescDTO), false) << "current state must be DISABLED after call to DisableComponent";
    EXPECT_FALSE(static_cast<bool>(sRef));
  }

  /**
   * Verify state changes for a component with initial state as DISABLED
   */
  TEST_F(tServiceComponent, testInitialDisabled) //testDS_TOI_2
  {
    std::string testBundleName("TestBundleDSTOI2");
    cppmicroservices::Bundle testBundle = StartTestBundle(testBundleName);
    scr::dto::ComponentDescriptionDTO compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent2");
    EXPECT_EQ(compDescDTO.name, "sample::ServiceComponent2") << "Name in the returned component description must be sample::ServiceComponentImpl";
    EXPECT_EQ(compDescDTO.defaultEnabled, false) << "Component's initial state in component description must be DISABLED";
    EXPECT_EQ(dsRuntimeService->IsComponentEnabled(compDescDTO), false) << "current state reported by the runtime service must match the initial state in component description";
    auto sRef = framework.GetBundleContext().GetServiceReference<test::Interface1>();
    EXPECT_FALSE(static_cast<bool>(sRef));
    auto fut = dsRuntimeService->EnableComponent(compDescDTO);
    EXPECT_NO_THROW(fut.get());
    EXPECT_EQ(dsRuntimeService->IsComponentEnabled(compDescDTO), true) << "current state must be ENABLED after call to EnableComponent";
    sRef = framework.GetBundleContext().GetServiceReference<test::Interface1>();
    EXPECT_TRUE(static_cast<bool>(sRef));
  }
}
