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
#include <atomic>
#include <thread>

namespace test
{

    class tMultipleRequiredConfigs : public tGenericDSAndCASuite
    {
      public:
        void
        SetUp() override
        {
            tGenericDSAndCASuite::SetUp();
            // Install the LogService bundle so SCRLogger output goes to stdout via spdlog.
            // InstallLib uses the library name; then we find and start by symbolic name.
            // ::test::InstallLib(context, "LogService");
            // for (auto& b : context.GetBundles())
            // {
            //     if (b.GetSymbolicName() == "log_service")
            //     {
            //         b.Start();
            //         break;
            //     }
            // }
            testBundle = ::test::InstallAndStartBundle(context, "TestBundleDSCA29");
            ASSERT_TRUE(testBundle);
        }

        void
        TearDown() override
        {
            if (configAdmin)
            {
                auto removeConfig = [this](std::string const& pid)
                {
                    try
                    {
                        auto cfg = configAdmin->GetConfiguration(pid);
                        cfg->Remove().get();
                    }
                    catch (...)
                    {
                    }
                };
                removeConfig(pid0);
                removeConfig(pid1);
            }

            if (testBundle && testBundle.GetState() == cppmicroservices::Bundle::STATE_ACTIVE)
            {
                testBundle.Stop();
            }

            tGenericDSAndCASuite::TearDown();
        }

        std::string const componentName = "sample::ServiceComponentCA29";
        std::string const pid0 = "sample::ServiceComponentCA29_pid0";
        std::string const pid1 = "sample::ServiceComponentCA29_pid1";
        cppmicroservices::Bundle testBundle;
    };

    /**
     * Stress test to expose a race in ConfigurationManager::Initialize().
     *
     * The race: Initialize() calls ListConfigurations() which returns a
     * shared_ptr to a ConfigurationImpl. It then calls GetProperties() on
     * that reference. If a concurrent Remove() sets removed=true between
     * those two calls, GetProperties() throws. The catch-all in Initialize()
     * swallows the exception and returns, leaving the component permanently
     * unsatisfied with no recovery path.
     *
     * To trigger: concurrently start the bundle (which triggers async
     * component initialization) and remove/re-create pid1.
     *
     * Run with --gtest_repeat=-1 to loop until failure.
     */
    TEST_F(tMultipleRequiredConfigs, testRequireTwoConfigsStress)
    {
        testBundle.Stop();

        constexpr int NUM_ITERATIONS = 20;
        constexpr auto SERVICE_TIMEOUT = std::chrono::milliseconds(5000);
        constexpr auto POLL_INTERVAL = std::chrono::milliseconds(5);

        for (int i = 0; i < NUM_ITERATIONS; ++i)
        {
            std::cout << "repetition: " << i << std::endl;
            // Pre-create both configs.
            {
                auto c0 = configAdmin->GetConfiguration(pid0);
                cppmicroservices::AnyMap p0(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
                p0["k"] = std::string("v0_") + std::to_string(i);
                auto f1 = c0->Update(p0);

                auto c1 = configAdmin->GetConfiguration(pid1);
                cppmicroservices::AnyMap p1(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
                p1["k"] = std::string("v1_") + std::to_string(i);
                auto f2 = c1->Update(p1);

                f1.get();
                f2.get();
            }

            // Spin-barrier so both threads release at the same time.
            std::atomic<int> barrier{0};

            // Thread 1: start the bundle (triggers async component init).
            std::thread t1([&]()
            {
                barrier.fetch_add(1, std::memory_order_release);
                while (barrier.load(std::memory_order_acquire) < 2) {}
                testBundle.Start();
            });

            // Thread 2: remove pid1 then immediately re-create it.
            std::thread t2([&]()
            {
                barrier.fetch_add(1, std::memory_order_release);
                while (barrier.load(std::memory_order_acquire) < 2) {}
                try { configAdmin->GetConfiguration(pid1)->Remove().get(); } catch (...) {}

                auto nc = configAdmin->GetConfiguration(pid1);
                cppmicroservices::AnyMap np(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
                np["k"] = std::string("v1_new_") + std::to_string(i);
                nc->Update(np).get();
            });

            t1.join();
            t2.join();

            // Poll for the service to appear.
            std::shared_ptr<test::CAInterface> instance;
            auto deadline = std::chrono::steady_clock::now() + SERVICE_TIMEOUT;
            while (!instance && std::chrono::steady_clock::now() < deadline)
            {
                instance = GetInstance<test::CAInterface>();
                if (!instance)
                {
                    std::this_thread::sleep_for(POLL_INTERVAL);
                }
            }

            ASSERT_TRUE(instance)
                << "Service did not appear within timeout on iteration " << i
                << ". Config notification for pid1 was likely lost during concurrent Remove/Update.";

            instance.reset();

            // Clean up for next iteration.
            testBundle.Stop();
            try { configAdmin->GetConfiguration(pid0)->Remove().get(); } catch (...) {}
            try { configAdmin->GetConfiguration(pid1)->Remove().get(); } catch (...) {}
        }
    }

} // namespace test
