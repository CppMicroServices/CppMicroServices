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

// White-box tests that reach into CoreBundleContext and its registries.
// GetPrivate(Bundle const&), CoreBundleContext, BundleRegistry and
// ServiceRegistry are all unexported framework internals; this test runs a
// real framework instance built from the objects linked into this executable.

#include "BundlePrivate.h"
#include "CoreBundleContext.h"

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/ServiceRegistrationBase.h"

#include "gtest/gtest.h"

#include <chrono>
#include <memory>

namespace cppmicroservices
{
    namespace
    {
        struct IUnitTestService
        {
            IUnitTestService() = default;
            IUnitTestService(IUnitTestService const&) = default;
            IUnitTestService(IUnitTestService&&) = default;
            IUnitTestService& operator=(IUnitTestService const&) = default;
            IUnitTestService& operator=(IUnitTestService&&) = default;
            virtual ~IUnitTestService() = default;
        };

        struct UnitTestService : IUnitTestService
        {
        };
    } // namespace

    TEST(CoreBundleContextUnitTest, ConstructorAppliesDefaultAndReadOnlyProperties)
    {
        FrameworkConfiguration config;
        config["user.property"] = std::string("kept");
        config[Constants::FRAMEWORK_VENDOR] = std::string("overridden?");
        config[Constants::FRAMEWORK_THREADING_SUPPORT] = std::string("bogus");

        // CoreBundleContext's constructor is private to FrameworkFactory, so
        // create it the normal way and reach in via GetPrivate().
        auto framework = FrameworkFactory().NewFramework(config);
        auto* ctx = GetPrivate(framework)->coreCtx;
        auto const& props = ctx->frameworkProperties;

        EXPECT_EQ("kept", any_cast<std::string>(props.at("user.property")));
        // Vendor and threading support are read-only: user values are replaced.
        EXPECT_EQ("CppMicroServices", any_cast<std::string>(props.at(Constants::FRAMEWORK_VENDOR)));
        EXPECT_NE("bogus", any_cast<std::string>(props.at(Constants::FRAMEWORK_THREADING_SUPPORT)));
        // Diagnostic logging defaults to off; working dir is always populated.
        EXPECT_FALSE(any_cast<bool>(props.at(Constants::FRAMEWORK_LOG)));
        EXPECT_FALSE(ctx->workingDir.empty());
    }

    TEST(CoreBundleContextUnitTest, EachContextGetsANewId)
    {
        auto a = FrameworkFactory().NewFramework();
        auto b = FrameworkFactory().NewFramework();
        EXPECT_LT(GetPrivate(a)->coreCtx->id, GetPrivate(b)->coreCtx->id);
    }

    TEST(CoreBundleContextUnitTest, SystemBundleIsTheOnlyBundleInAFreshFramework)
    {
        auto framework = FrameworkFactory().NewFramework();
        framework.Start();

        auto* coreCtx = GetPrivate(framework)->coreCtx;
        ASSERT_NE(nullptr, coreCtx);

        auto bundles = coreCtx->bundleRegistry.GetBundles();
        ASSERT_EQ(1u, bundles.size());
        EXPECT_EQ(0, bundles.front()->id);
        EXPECT_EQ(Constants::SYSTEM_BUNDLE_SYMBOLICNAME, bundles.front()->symbolicName);
        EXPECT_EQ(GetPrivate(framework), bundles.front());

        framework.Stop();
        framework.WaitForStop(std::chrono::milliseconds::zero());
    }

    TEST(CoreBundleContextUnitTest, PublicRegistrationIsVisibleInInternalServiceRegistry)
    {
        auto framework = FrameworkFactory().NewFramework();
        framework.Start();
        auto* coreCtx = GetPrivate(framework)->coreCtx;

        std::vector<ServiceRegistrationBase> regs;
        coreCtx->services.Get(us_service_interface_iid<IUnitTestService>(), regs);
        EXPECT_TRUE(regs.empty());

        auto reg = framework.GetBundleContext().RegisterService<IUnitTestService>(std::make_shared<UnitTestService>());

        coreCtx->services.Get(us_service_interface_iid<IUnitTestService>(), regs);
        ASSERT_EQ(1u, regs.size());
        ServiceReferenceBase const publicRef = reg.GetReference();
        EXPECT_TRUE(publicRef == regs.front().GetReference(us_service_interface_iid<IUnitTestService>()));

        reg.Unregister();
        regs.clear();
        coreCtx->services.Get(us_service_interface_iid<IUnitTestService>(), regs);
        EXPECT_TRUE(regs.empty());

        framework.Stop();
        framework.WaitForStop(std::chrono::milliseconds::zero());
    }
} // namespace cppmicroservices
