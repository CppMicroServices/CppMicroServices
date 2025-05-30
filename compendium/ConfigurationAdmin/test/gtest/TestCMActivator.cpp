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

#include <future>

#include "gmock/gmock.h"

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleImport.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "../../src/CMActivator.hpp"

#define str(s)  #s
#define xstr(s) str(s)

namespace cppmicroservices
{
    namespace cmimpl
    {
        // The fixture for testing class CMActivator.
        class TestCMActivator : public ::testing::Test
        {
          protected:
            TestCMActivator() : framework(cppmicroservices::FrameworkFactory().NewFramework()) {}
            ~TestCMActivator() override = default;

            void
            SetUp() override
            {
                framework.Start();
            }

            void
            TearDown() override
            {
                framework.Stop();
                framework.WaitForStop(std::chrono::milliseconds::zero());
            }

            cppmicroservices::Framework&
            GetFramework()
            {
                return framework;
            }

          private:
            cppmicroservices::Framework framework;
        };

        TEST_F(TestCMActivator, VerifyConfigAdmin)
        {
            EXPECT_NO_THROW({
                auto bundleContext = GetFramework().GetBundleContext();
                auto allBundles = bundleContext.GetBundles();
                EXPECT_EQ(allBundles.size(), static_cast<size_t>(2));
                cppmicroservices::Bundle selfBundle;
                for (const auto& bundle : allBundles)
                {
                    if (xstr(US_BUNDLE_NAME) == bundle.GetSymbolicName())
                    {
                        selfBundle = bundle;
                        break;
                    }
                }
                ASSERT_TRUE(selfBundle);
                selfBundle.Start();
                EXPECT_EQ(selfBundle.GetState(), cppmicroservices::Bundle::STATE_ACTIVE);
                auto selfContext = selfBundle.GetBundleContext();
                EXPECT_EQ(selfBundle.GetRegisteredServices().size(), static_cast<size_t>(1));
                auto serviceRef
                    = bundleContext.GetServiceReference<cppmicroservices::service::cm::ConfigurationAdmin>();
                auto service = bundleContext.GetService<cppmicroservices::service::cm::ConfigurationAdmin>(serviceRef);
                EXPECT_NE(service.get(), nullptr);
            });
        }

        TEST_F(TestCMActivator, VerifyConcurrentStartStop)
        {
            auto bundleContext = GetFramework().GetBundleContext();
            auto allBundles = bundleContext.GetBundles();
            EXPECT_EQ(allBundles.size(), static_cast<size_t>(2));
            cppmicroservices::Bundle selfBundle;
            for (auto bundle : allBundles)
            {
                if (xstr(US_BUNDLE_NAME) == bundle.GetSymbolicName())
                {
                    selfBundle = bundle;
                    break;
                }
            }
            ASSERT_TRUE(selfBundle);
            std::promise<void> go;
            std::shared_future<void> ready(go.get_future());
            int numCalls = 50;
            std::vector<std::promise<void>> readies(numCalls);
            std::vector<std::future<void>> bundle_state_changes(numCalls);
            EXPECT_NO_THROW({
                for (int i = 0; i < numCalls; ++i)
                {
                    bundle_state_changes[i] = std::async(std::launch::async,
                                                         [&selfBundle, ready, &readies, i]()
                                                         {
                                                             readies[i].set_value();
                                                             ready.wait();
                                                             ((i % 2) ? selfBundle.Start() : selfBundle.Stop());
                                                         });
                }

                for (int i = 0; i < numCalls; i++)
                {
                    readies[i].get_future().wait();
                }
                go.set_value();
                for (int i = 0; i < numCalls; i++)
                {
                    bundle_state_changes[i].wait();
                }
            });
        }
    } // namespace cmimpl
} // namespace cppmicroservices

CPPMICROSERVICES_IMPORT_BUNDLE(US_BUNDLE_NAME);
