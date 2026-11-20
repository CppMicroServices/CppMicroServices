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

// Pins down the system bundle behaviour that usFrameworkUnitTests relies on.
//
// The shipping CppMicroServices library embeds a "system_bundle/manifest.json"
// resource zip. This executable deliberately does NOT embed it:
//
//  * The system bundle never gets a BundleArchive; its identity and headers
//    are built in code (BundlePrivate / FrameworkPrivate constructors), so
//    the embedded manifest is never read for bundle 0.
//  * In shared builds, BundleRegistry::Install only skips a "system_bundle"
//    zip entry when US_BUILD_SHARED_LIBS is *not* defined. If this exe
//    embedded the framework zip, CoreBundleContext::Init would try to
//    auto-install the exe, and Framework::Start() throws: "a bundle with same
//    symbolic name and version is already installed (system_bundle, ...)".
//    (Verified by temporarily embedding the zip; every test that starts a
//    framework fails.)
//
//  * Note also that usFunctionEmbedResources(ZIP_ARCHIVES <target>) silently
//    ignores non-STATIC targets, so in shared builds simply passing
//    CppMicroServices there does nothing at all.
//
// These tests fail if either assumption stops holding.

#include "BundlePrivate.h"
#include "CoreBundleContext.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleResource.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkConfig.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/util/FileSystem.h"

#include "gtest/gtest.h"

#include <chrono>

namespace cppmicroservices
{
    namespace
    {
        class SystemBundleUnitTest : public ::testing::Test
        {
          protected:
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

            Framework&
            GetFramework()
            {
                return framework;
            }

          private:
            Framework framework = FrameworkFactory().NewFramework();
        };
    } // namespace

    TEST_F(SystemBundleUnitTest, SystemBundleHasNoArchive)
    {
        auto& framework = GetFramework();
        EXPECT_EQ(nullptr, GetPrivate(framework)->GetBundleArchive());
        EXPECT_FALSE(framework.GetResource("/manifest.json"));
        EXPECT_TRUE(framework.FindResources("/", "*", true).empty());
    }

    TEST_F(SystemBundleUnitTest, HeadersComeFromCodeNotFromAnEmbeddedManifest)
    {
        auto& framework = GetFramework();
        auto const& headers = framework.GetHeaders();
        EXPECT_EQ(Constants::SYSTEM_BUNDLE_SYMBOLICNAME,
                  any_cast<std::string>(headers.at(Constants::BUNDLE_SYMBOLICNAME)));
        EXPECT_EQ(US_FRAMEWORK_VERSION_STR, any_cast<std::string>(headers.at(Constants::BUNDLE_VERSION)));
        EXPECT_EQ(US_FRAMEWORK_VERSION_STR, framework.GetVersion().ToString());
        EXPECT_EQ(Constants::SYSTEM_BUNDLE_LOCATION, framework.GetLocation());
    }

    TEST_F(SystemBundleUnitTest, TestExecutableIsNotAutoInstalledAsABundle)
    {
        auto& framework = GetFramework();
        auto* coreCtx = GetPrivate(framework)->coreCtx;

        EXPECT_TRUE(coreCtx->bundleRegistry.GetBundles(util::GetExecutablePath()).empty());
        EXPECT_EQ(1u, framework.GetBundleContext().GetBundles().size());
        EXPECT_TRUE(framework.GetBundleContext().GetBundles(util::GetExecutablePath()).empty());
    }
} // namespace cppmicroservices
