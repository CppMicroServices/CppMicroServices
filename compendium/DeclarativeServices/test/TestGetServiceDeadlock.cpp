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
TEST_F(tGenericDSSuite, TestGetServiceInConstructorHang)
{
  auto bundleContext = framework.GetBundleContext();

  test::InstallAndStartBundle(bundleContext, "TestBundleDSTOI10");
  test::InstallAndStartBundle(bundleContext, "TestBundleDSTOI20");

  auto sRef = bundleContext.GetServiceReference<test::Interface4>();
  auto service = bundleContext.GetService<test::Interface4>(sRef);
  ASSERT_TRUE(service->GetService());
}
}
