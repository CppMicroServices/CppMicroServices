#include <chrono>

#include <gtest/gtest.h>

#include "../../src/SCRExtensionRegistry.hpp"
#include "../../src/manager/ComponentConfigurationImpl.hpp"
#include "../../src/manager/ReferenceManager.hpp"
#include "../../src/manager/SingletonComponentConfiguration.hpp"

#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "../TestUtils.hpp"
#include "Mocks.hpp"

namespace scr = cppmicroservices::service::component::runtime;

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

#include <TestInterfaces/Interfaces.hpp>
#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/ServiceTracker.h>
#include <cppmicroservices/servicecomponent/ComponentConstants.hpp>
#include <cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp>

#include "../TestUtils.hpp"

namespace test
{

    TEST(TestCircularReference, circularReferenceOptionalTest)
    {
        auto framework = cppmicroservices::FrameworkFactory().NewFramework();
        framework.Start();
        EXPECT_TRUE(framework);

        auto dsPluginPath = test::GetDSRuntimePluginFilePath();
        auto context = framework.GetBundleContext();

        test::InstallAndStartDS(context);

        // The names of the bundles do matter here. The bundle containing the dependency MUST
        // be stopped after the one providing the dependency. CppMicroServices stores bundles
        // in sorted order by path.

        auto bundleA = test::InstallAndStartBundle(context, "TestBundleCircular01");
        ASSERT_TRUE(bundleA);
        bundleA.Start();

        auto bundleB = test::InstallAndStartBundle(context, "TestBundleCircular02");
        ASSERT_TRUE(bundleB);
        bundleB.Start();

        auto refA = context.GetServiceReference<test::CircularInterface1>();

        auto refB = context.GetServiceReference<test::CircularInterface2>();

        // assert that references are invalid with unsatisfied configuration
        ASSERT_EQ(refA.operator bool(), false);
        ASSERT_EQ(refB.operator bool(), false);

        framework.Stop();
        framework.WaitForStop(std::chrono::milliseconds::zero());
    }

} // namespace test