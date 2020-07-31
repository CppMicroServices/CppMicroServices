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

#include <chrono>

#include <gtest/gtest.h>

#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/ServiceTracker.h>
#include <cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp>
#include <cppmicroservices/servicecomponent/ComponentConstants.hpp>
#include <TestInterfaces/Interfaces.hpp>

#include "TestUtils.hpp"

namespace test {
/**
 * Test for the case where one bundle both provides and depends on a service
 * of the same interface AND that bundle is stopped after the bundle
 * providing the service dependency.
 *
 *
 * A failure will result in a deadlock.
 */
TEST(tServiceComponentRuntime, testReferenceSelfSatisfyDeadlock) {
  auto framework = cppmicroservices::FrameworkFactory().NewFramework();
  framework.Start();
  EXPECT_TRUE(framework);
    
  auto dsPluginPath = test::GetDSRuntimePluginFilePath();
  auto dsbundles = framework.GetBundleContext().InstallBundles(dsPluginPath);
  for (auto& bundle : dsbundles) {
    bundle.Start();
  }

  // The names of the bundles do matter here. The bundle containing the dependency MUST
  // be stopped after the one providing the dependency. CppMicroServices stores bundles
  // in sorted order by path.
  auto bundleB = test::InstallAndStartBundle(framework.GetBundleContext(), "TestBundleDSb");
  ASSERT_TRUE(bundleB);
  bundleB.Start();
  auto bundleA = test::InstallAndStartBundle(framework.GetBundleContext(), "TestBundleDSa");
  ASSERT_TRUE(bundleA);
  bundleA.Start();

  framework.Stop();
  framework.WaitForStop(std::chrono::milliseconds::zero());
}
} // namespace test