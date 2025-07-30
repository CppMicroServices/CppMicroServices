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

#include "IDictionaryService/IDictionaryService.hpp"
#include "ISpellCheckService/ISpellCheckService.hpp"
#include "TestFixture.hpp"
#include "../TestUtils.hpp"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/ServiceEvent.h"
#include "gtest/gtest.h"

namespace test
{
    class TestDictionaryFixture : public ::testing::Test
    {
      public:
        TestDictionaryFixture() : ::testing::Test(), framework(cppmicroservices::FrameworkFactory().NewFramework()) {}

        void
        SetUp() override 
        {
            framework.Start();
            context = framework.GetBundleContext();

#if defined(US_BUILD_SHARED_LIBS)
            auto dsPluginPath = test::GetDSRuntimePluginFilePath();
            auto dsbundles = context.InstallBundles(dsPluginPath);
            for (auto& bundle : dsbundles)
            {
                bundle.Start();
            }

            test::InstallLib(context, "DSFrenchDictionary");
            test::InstallLib(context, "EnglishDictionary");
            test::InstallLib(context, "DSSpellChecker");
#endif

#ifndef US_BUILD_SHARED_LIBS
            auto dsbundles = context.GetBundles();
            for (auto& bundle : dsbundles)
            {
                try
                {
                    bundle.Start();
                }
                catch (std::exception& e)
                {
                    std::cerr << "    " << e.what();
                }
                std::cerr << std::endl;
            }
#endif
            auto sRef = context.GetServiceReference<scr::ServiceComponentRuntime>();
            ASSERT_TRUE(sRef);
            dsRuntimeService = context.GetService<scr::ServiceComponentRuntime>(sRef);
            ASSERT_TRUE(dsRuntimeService);
        }

        void
        TearDown() override
        {
            framework.Stop();
            framework.WaitForStop(std::chrono::milliseconds::zero());
        }

        cppmicroservices::Bundle
        GetTestBundle(std::string const& symbolicName)
        {
            auto bundles = context.GetBundles();

            for (auto& bundle : bundles)
            {
                auto bundleSymbolicName = bundle.GetSymbolicName();
                if (symbolicName == bundleSymbolicName)
                {
                    return bundle;
                }
            }
            return cppmicroservices::Bundle();
        }

        cppmicroservices::Bundle
        StartTestBundle(std::string const& symName)
        {
            cppmicroservices::Bundle testBundle = GetTestBundle(symName);
            EXPECT_EQ(static_cast<bool>(testBundle), true);
            testBundle.Start();
            EXPECT_EQ(testBundle.GetState(), cppmicroservices::Bundle::STATE_ACTIVE)
                << " failed to start bundle with symbolic name" + symName;
            return testBundle;
        }

      protected:
        std::shared_ptr<scr::ServiceComponentRuntime> dsRuntimeService;
        cppmicroservices::Framework framework;
        cppmicroservices::BundleContext context;

    };
    /**
     * Verify Spellcheck service works as expected with a Dictionary service from a
     * DS bundle and a non-DS bundle
     */
    TEST_F(TestDictionaryFixture, testDictionaryExample) // DS_TOI_50
    {

        auto ctxt = framework.GetBundleContext();

        // try DS spellchecker with DS dictionary
        EXPECT_TRUE(ctxt.GetServiceReferences<test::IDictionaryService>().empty());
        auto frenchDictDSBundle = StartTestBundle("DSFrenchDictionary");
        EXPECT_EQ(ctxt.GetServiceReferences<test::IDictionaryService>().size(), 1ul);
        EXPECT_TRUE(ctxt.GetServiceReferences<test::ISpellCheckService>().empty());
        auto spellcheckerBundle = StartTestBundle("DSSpellChecker");
        EXPECT_FALSE(ctxt.GetServiceReferences<test::ISpellCheckService>().empty());
        auto spellCheckRef = ctxt.GetServiceReference<test::ISpellCheckService>();
        auto service = ctxt.GetService<test::ISpellCheckService>(spellCheckRef);
        ASSERT_NE(service, nullptr);
        auto spellCheckServiceId = GetServiceId(spellCheckRef);
        EXPECT_TRUE(service->Check("bienvenue au tutoriel micro services").empty());
        EXPECT_FALSE(service->Check("bienvenue au tutoriel microservices").empty());
        EXPECT_TRUE(service->Check("bienvenue au").empty()); // verify partial string

        // register service listener and wait for SERVICE_UNREGISTERING event for SpellCheck service
        std::mutex mtx;
        std::condition_variable cv;
        auto token = ctxt.AddServiceListener(
            [&](cppmicroservices::ServiceEvent const& evt)
            {
                if ((evt.GetType() & cppmicroservices::ServiceEvent::SERVICE_UNREGISTERING)
                    && (spellCheckServiceId == GetServiceId(evt.GetServiceReference())))
                {
                    std::unique_lock<std::mutex> lk(mtx);
                    cv.notify_all();
                }
            });
        frenchDictDSBundle.Stop(); // remove the dependency for spellchecker
        {
            std::unique_lock<std::mutex> lk(mtx);
            bool result = cv.wait_for(
                lk,
                std::chrono::milliseconds(30000),
                [&ctxt]() -> bool
                { return static_cast<bool>(ctxt.GetServiceReference<test::ISpellCheckService>()) == false; });
            ctxt.RemoveListener(std::move(token));
            ASSERT_TRUE(result);
            EXPECT_TRUE(ctxt.GetServiceReferences<test::ISpellCheckService>().empty());
            spellCheckRef = ctxt.GetServiceReference<test::ISpellCheckService>();
            EXPECT_FALSE(static_cast<bool>(spellCheckRef));
        }

        spellcheckerBundle.Stop();

        // try the DS spell checker with a non-DS dictionary
        spellcheckerBundle.Start(); // restart the bundle to re-use it, since the
                                    // default binding policy is static reluctant.
        EXPECT_TRUE(ctxt.GetServiceReferences<test::ISpellCheckService>().empty());
        EXPECT_TRUE(ctxt.GetServiceReferences<test::IDictionaryService>().empty());

        // register service listener and wait for SERVICE_REGISTERED event for SpellCheck service
        std::mutex mtx1;
        std::condition_variable cv1;
        auto token1 = ctxt.AddServiceListener(
            [&](cppmicroservices::ServiceEvent const& evt)
            {
                if (evt.GetType() & cppmicroservices::ServiceEvent::SERVICE_REGISTERED
                    && evt.GetServiceReference().GetBundle() == spellcheckerBundle)
                {
                    {
                        std::unique_lock<std::mutex> lk(mtx1);
                    }
                    cv1.notify_all();
                }
            });

        auto englishDictDSBundle = StartTestBundle("EnglishDictionary");
        EXPECT_EQ(ctxt.GetServiceReferences<test::IDictionaryService>().size(), 1ul);
        {
            std::unique_lock<std::mutex> lk(mtx1);
            auto result = cv1.wait_for(
                lk,
                std::chrono::milliseconds(30000),
                [&ctxt]() -> bool { return static_cast<bool>(ctxt.GetServiceReference<test::ISpellCheckService>()); });
            ctxt.RemoveListener(std::move(token1));
            ASSERT_TRUE(result);
            spellCheckRef = ctxt.GetServiceReference<test::ISpellCheckService>();
            ASSERT_TRUE(static_cast<bool>(spellCheckRef));
            service = ctxt.GetService<test::ISpellCheckService>(spellCheckRef);
        }

        ASSERT_NE(service, nullptr);
        EXPECT_TRUE(service->Check("welcome to micro services tutorial").empty());
        auto misspelledWords = service->Check("welcome to microservices tutorial page");
        EXPECT_FALSE(misspelledWords.empty());
        auto expectedWords = { "microservices", "page" };
        EXPECT_TRUE(std::equal(misspelledWords.begin(), misspelledWords.end(), expectedWords.begin()));
        EXPECT_TRUE(service->Check("welcome micro").empty()); // verify partial string
    }
} // namespace test
