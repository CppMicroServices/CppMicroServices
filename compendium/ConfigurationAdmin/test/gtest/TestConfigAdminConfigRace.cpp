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

#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/cm/ConfigurationAdmin.hpp>
#include <cppmicroservices/util/FileSystem.h>

#include "TestInterfaces/Interfaces.hpp"

#include "TestFixtures.hpp"

#include <atomic>
#include <chrono>
#include <future>
#include <string>
#include <thread>

namespace
{
    std::string
    InstallAndStartBundle(cppmicroservices::BundleContext& ctx, std::string const& bundleName)
    {
        std::string path = cppmicroservices::testing::LIB_PATH + cppmicroservices::util::DIR_SEP + US_LIB_PREFIX
                           + bundleName + US_LIB_POSTFIX + US_LIB_EXT;

#if defined(US_BUILD_SHARED_LIBS)
        auto bundles = ctx.InstallBundles(path);
        for (auto& b : bundles)
        {
            b.Start();
        }
#endif
        return path;
    }
} // namespace

/**
 * Tests that a race condition between removing a required Configuration and
 * a concurrent GetService call does not result in a component being
 * constructed without its required configuration properties.
 *
 * The race window is in ComponentConfigurationImpl::ConfigChangedState where
 * UpdateMergedProperties removes the config properties BEFORE Deactivate()
 * unregisters the service. A concurrent GetService during this window will
 * call DoCreate -> GetProperties and get properties without the required
 * configuration.
 */
TEST_F(tGenericDSAndCASuite, testGetServiceDuringConfigRemovalRace)
{
    std::string const configPid { "sample::ServiceComponentCA29" };

    InstallAndStartBundle(context, "TestBundleDSCA29");

    int const iterations = 25;
    int constructedWithoutConfig = 0;

    for (int i = 0; i < iterations; ++i)
    {
        // Create the configuration so the component becomes satisfied
        auto configuration = configAdmin->GetConfiguration(configPid);
        auto fut = configuration->Update(
            cppmicroservices::AnyMap { cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS,
                                       { { "testProperty", std::string("value") } } });
        fut.get();

        // Wait until the service is available (component is SATISFIED/ACTIVE)
        auto sr = context.GetServiceReference<test::CAInterface>();
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (!sr && std::chrono::steady_clock::now() < deadline)
        {
            std::this_thread::yield();
            sr = context.GetServiceReference<test::CAInterface>();
        }
        ASSERT_TRUE(sr) << "Service never became available after config update, iteration " << i;

        // Race: remove the configuration while concurrently trying to GetService.
        // The Remove() is asynchronous and will trigger ConfigChangedState.
        // Meanwhile we hammer GetService to try to hit the window between
        // UpdateMergedProperties and Deactivate.
        std::promise<void> go;
        std::shared_future<void> ready(go.get_future());

        auto removeFuture = std::async(std::launch::async,
                                       [&ready, &configuration]()
                                       {
                                           ready.wait();
                                           configuration->Remove();
                                       });

        auto getServiceFuture = std::async(std::launch::async,
                                           [&ready, this]()
                                           {
                                               ready.wait();
                                               for (int j = 0; j < 50; ++j)
                                               {
                                                   auto ref = context.GetServiceReference<test::CAInterface>();
                                                   if (ref)
                                                   {
                                                       auto svc = context.GetService<test::CAInterface>(ref);
                                                       if (svc)
                                                       {
                                                           auto props = svc->GetProperties();
                                                           if (props.find("testProperty") == props.end())
                                                           {
                                                               return true;
                                                           }
                                                       }
                                                   }
                                                   std::this_thread::yield();
                                               }
                                               return false;
                                           });

        go.set_value();
        removeFuture.get();
        bool raceHit = getServiceFuture.get();
        if (raceHit)
        {
            ++constructedWithoutConfig;
        }

        // Wait for the service to become unavailable (component deactivated)
        deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        sr = context.GetServiceReference<test::CAInterface>();
        while (sr && std::chrono::steady_clock::now() < deadline)
        {
            std::this_thread::yield();
            sr = context.GetServiceReference<test::CAInterface>();
        }
    }

    // If the bug is present, constructedWithoutConfig will be > 0 at least some of the time.
    // Once the fix is in place, this should always be 0.
    EXPECT_EQ(constructedWithoutConfig, 0)
        << "Component was constructed " << constructedWithoutConfig << " time(s) out of " << iterations
        << " iterations without its required configuration property. "
        << "This indicates the race condition between ConfigChangedState and GetService.";
}
