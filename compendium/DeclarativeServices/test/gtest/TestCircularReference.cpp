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

#include "../../src/SCRExtensionRegistry.hpp"
#include "../../src/manager/ComponentConfigurationImpl.hpp"
#include "../../src/manager/ReferenceManager.hpp"
#include "../../src/manager/SingletonComponentConfiguration.hpp"
#include "../TestUtils.hpp"
#include "cppmicroservices/logservice/LogService.hpp"

#include "Mocks.hpp"

namespace scr = cppmicroservices::service::component::runtime;
namespace test
{
    class TestCircularReference : public ::testing::Test
    {
      protected:
        TestCircularReference()
            : framework(cppmicroservices::FrameworkFactory().NewFramework())
            , logger(std::make_shared<::testing::NiceMock<cppmicroservices::scrimpl::MockLogger>>())
        {
        }
        virtual ~TestCircularReference() = default;

        virtual void
        SetUp()
        {
            framework.Start();
            context = framework.GetBundleContext();
            EXPECT_TRUE(framework);
        }

        virtual void
        ExpectCircular(std::vector<std::string> msgs, int times)
        {
            std::vector<testing::PolymorphicMatcher<testing::internal::HasSubstrMatcher<std::string>>> tmp;
            for (auto msg : msgs)
            {
                tmp.push_back(testing::HasSubstr(msg));
            }
            // set logging expectations
            auto CircularReference = testing::AllOfArray(tmp.begin(), tmp.end());
            EXPECT_CALL(*logger,
                        Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR, CircularReference, testing::_))
                .Times(times);
        }

        virtual void
        RegisterLoggerStartBundle(std::string bundleName)
        {
            context.RegisterService<LogService>(logger);

            test::InstallAndStartConfigAdmin(context);
            test::InstallAndStartDS(context);

            ASSERT_TRUE(test::InstallAndStartBundle(context, bundleName));
        }

        virtual void
        TearDown()
        {
            framework.Stop();
            framework.WaitForStop(std::chrono::milliseconds::zero());
        }

        cppmicroservices::Framework framework;
        std::shared_ptr<::testing::NiceMock<cppmicroservices::scrimpl::MockLogger>> logger;
        cppmicroservices::BundleContext context;
    };

    // build graph with complex circuluar reference
    /*
    [01, 02]       [03]    [05, 06]  [04] 07
     ||   \\        ||        ||      ||
     04    03       05        01      07
    */
    TEST_F(TestCircularReference, basicGraph)
    {
        std::vector<std::string> vectorSubs { "Circular Reference: ",
                                              "(sample::ServiceComponentComplexCircular56: test::DSGraph05)",
                                              "(sample::ServiceComponentComplexCircular12: test::DSGraph01)",
                                              "(sample::ServiceComponentComplexCircular3: test::DSGraph03)" };
        ExpectCircular(vectorSubs, 1);
        RegisterLoggerStartBundle("TestBundleCircularBasic");

        auto ref1 = context.GetServiceReference<test::DSGraph01>();
        auto ref2 = context.GetServiceReference<test::DSGraph02>();
        auto ref3 = context.GetServiceReference<test::DSGraph03>();
        auto ref4 = context.GetServiceReference<test::DSGraph04>();
        auto ref5 = context.GetServiceReference<test::DSGraph05>();
        auto ref6 = context.GetServiceReference<test::DSGraph06>();
        auto ref7 = context.GetServiceReference<test::DSGraph07>();

        // assert that references are invalid with unsatisfied configuration
        ASSERT_EQ(ref1.operator bool(), false);
        ASSERT_EQ(ref2.operator bool(), false);
        ASSERT_EQ(ref3.operator bool(), false);
        ASSERT_EQ(ref5.operator bool(), false);
        ASSERT_EQ(ref6.operator bool(), false);
        ASSERT_EQ(ref7.operator bool(), true);
        ASSERT_EQ(ref4.operator bool(), true);
    }

    // build graph with complex circuluar reference
    /*
     01--03   04--06--07
     || /     || //
      02       05
    */
    TEST_F(TestCircularReference, twoSmallGraph)
    {
        std::vector<std::string> vectorSubs { "Circular Reference: ",
                                              "(sample::ServiceComponentTwoCircles6: test::DSGraph06)",
                                              "(sample::ServiceComponentTwoCircles4: test::DSGraph04)",
                                              "(sample::ServiceComponentTwoCircles5: test::DSGraph05)" };
        ExpectCircular(vectorSubs, 1);
        RegisterLoggerStartBundle("TestBundleCircularDouble");

        auto ref1 = context.GetServiceReference<test::DSGraph01>();
        auto ref2 = context.GetServiceReference<test::DSGraph02>();
        auto ref3 = context.GetServiceReference<test::DSGraph03>();
        auto ref4 = context.GetServiceReference<test::DSGraph04>();
        auto ref5 = context.GetServiceReference<test::DSGraph05>();
        auto ref6 = context.GetServiceReference<test::DSGraph06>();
        auto ref7 = context.GetServiceReference<test::DSGraph07>();

        // assert that references are invalid with unsatisfied configuration
        ASSERT_EQ(ref1.operator bool(), true);
        ASSERT_EQ(ref2.operator bool(), true);
        ASSERT_EQ(ref3.operator bool(), true);
        ASSERT_EQ(ref4.operator bool(), false);
        ASSERT_EQ(ref5.operator bool(), false);
        ASSERT_EQ(ref6.operator bool(), false);
        ASSERT_EQ(ref7.operator bool(), true);
    }

    // build graph with complex circuluar reference
    TEST_F(TestCircularReference, optionalAndRequiredPath)
    {
        std::vector<std::string> vectorSubs { "Circular Reference: ",
                                              "(sample::ServiceComponentOptReq4: test::DSGraph04)",
                                              "(sample::ServiceComponentOptReq1: test::DSGraph01)",
                                              "(sample::ServiceComponentOptReq2: test::DSGraph02)",
                                              "(sample::ServiceComponentOptReq3: test::DSGraph03)" };
        ExpectCircular(vectorSubs, 1);
        RegisterLoggerStartBundle("TestBundleCircularOptReq");

        auto ref1 = context.GetServiceReference<test::DSGraph01>();
        auto ref2 = context.GetServiceReference<test::DSGraph02>();
        auto ref3 = context.GetServiceReference<test::DSGraph03>();
        auto ref4 = context.GetServiceReference<test::DSGraph04>();

        // assert that references are invalid with unsatisfied configuration
        ASSERT_EQ(ref1.operator bool(), true);
        ASSERT_EQ(ref2.operator bool(), true);
        ASSERT_EQ(ref3.operator bool(), true);
        ASSERT_EQ(ref4.operator bool(), true);
    }

    // build graph with complex circuluar reference
    /*
      [01]  [02]  [02]
       ||    ||    ||
      [02]  [02]  [01]
    */
    TEST_F(TestCircularReference, selfDependencyCycle)
    {
        std::vector<std::string> vectorSubs { "Circular Reference: ",
                                              "(sample::ServiceComponentSelfDep3: test::DSGraph02)",
                                              "(sample::ServiceComponentSelfDep1: test::DSGraph01)" };
        ExpectCircular(vectorSubs, 1);
        RegisterLoggerStartBundle("TestBundleCircularSelfDep");

        auto ref1 = context.GetServiceReference<test::DSGraph01>();
        auto ref2 = context.GetServiceReference("sample::ServiceComponentSelfDep2");
        auto ref3 = context.GetServiceReference("sample::ServiceComponentSelfDep3");

        // assert that references are invalid with unsatisfied configuration
        ASSERT_EQ(ref1.operator bool(), false);
        ASSERT_EQ(ref2.operator bool(), false);
        ASSERT_EQ(ref3.operator bool(), false);
    }
} // namespace test