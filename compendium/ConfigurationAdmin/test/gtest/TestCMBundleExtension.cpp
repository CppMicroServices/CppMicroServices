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

#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>

#include "../../src/CMBundleExtension.hpp"
#include "../../src/CMConstants.hpp"
#include "Mocks.hpp"

#define str(s)  #s
#define xstr(s) str(s)

namespace cppmicroservices
{
    namespace cmimpl
    {
        // The fixture for testing class CMBundleExtension.
        class TestCMBundleExtension : public ::testing::Test
        {
          protected:
            TestCMBundleExtension() : framework(cppmicroservices::FrameworkFactory().NewFramework()) {}
            ~TestCMBundleExtension() = default;

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

        TEST_F(TestCMBundleExtension, CtorInvalidArgs)
        {
            AnyMap headers(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
            headers["test"] = std::string { "value" };
            auto mockConfigAdmin = std::make_shared<MockConfigurationAdminPrivate>();
            auto fakeLogger = std::make_shared<FakeLogger>();
            EXPECT_THROW({ CMBundleExtension bundleExt(BundleContext(), headers, mockConfigAdmin, fakeLogger); },
                         std::invalid_argument);
            EXPECT_THROW(
                {
                    CMBundleExtension bundleExt(GetFramework().GetBundleContext(),
                                                AnyMap { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS },
                                                mockConfigAdmin,
                                                fakeLogger);
                },
                std::invalid_argument);
            EXPECT_THROW(
                { CMBundleExtension bundleExt(GetFramework().GetBundleContext(), headers, nullptr, fakeLogger); },
                std::invalid_argument);
            EXPECT_THROW(
                { CMBundleExtension bundleExt(GetFramework().GetBundleContext(), headers, mockConfigAdmin, nullptr); },
                std::invalid_argument);
            EXPECT_THROW(
                {
                    CMBundleExtension bundleExt(GetFramework().GetBundleContext(),
                                                headers,
                                                mockConfigAdmin,
                                                fakeLogger);
                },
                std::runtime_error);
        }

        TEST_F(TestCMBundleExtension, CtorWithValidArgs)
        {
            auto bundles = GetFramework().GetBundleContext().GetBundles();
            auto thisBundleItr = std::find_if(bundles.begin(),
                                              bundles.end(),
                                              [](cppmicroservices::Bundle const& bundle)
                                              { return (xstr(US_BUNDLE_NAME) == bundle.GetSymbolicName()); });
            auto thisBundle = thisBundleItr != bundles.end() ? *thisBundleItr : cppmicroservices::Bundle();
            ASSERT_TRUE(static_cast<bool>(thisBundle));
            auto const& cm = cppmicroservices::ref_any_cast<AnyMap>(thisBundle.GetHeaders().at("cm_test_0"));

            auto mockConfigAdmin = std::make_shared<MockConfigurationAdminPrivate>();
            std::vector<ConfigurationAddedInfo> returnValue {
                {std::string { "test" }, 1u, static_cast<std::uintptr_t>(42u)}
            };
            EXPECT_CALL(*mockConfigAdmin, AddConfigurations(testing::_))
                .Times(2)
                .WillOnce(testing::Throw(std::runtime_error("Failed to add component")))
                .WillOnce(testing::Return(returnValue));
            EXPECT_CALL(*mockConfigAdmin, RemoveConfigurations(returnValue)).Times(1);
            auto fakeLogger = std::make_shared<FakeLogger>();
            EXPECT_THROW(
                { CMBundleExtension bundleExt(GetFramework().GetBundleContext(), cm, mockConfigAdmin, fakeLogger); },
                std::runtime_error);
            EXPECT_NO_THROW(
                { CMBundleExtension bundleExt(GetFramework().GetBundleContext(), cm, mockConfigAdmin, fakeLogger); });
        }
    } // namespace cmimpl
} // namespace cppmicroservices
