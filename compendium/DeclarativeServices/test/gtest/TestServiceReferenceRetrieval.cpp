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
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/ServiceObjects.h"
#include "cppmicroservices/ServiceReference.h"
#include "gtest/gtest.h"

#include "TestInterfaces/Interfaces.hpp"

namespace test
{
    struct int1
    {
        virtual int getValue() const = 0;
        virtual ~int1() {}
    };

    struct TestServiceA : public int1
    {
        TestServiceA(int val) : id(val) {}
        int
        getValue() const
        {
            return id;
        }

      private:
        int id;
    };
    /*
     * Tests that if a configuration object is created programmatically before
     * the service that is dependent on the configuration object is installed
     * and started, the service is resolved as soon as it is started.
     */
    TEST_F(tServiceComponent, testServiceReferenceRetrievalForDSAndManReg)
    {
        cppmicroservices::BundleContext ctx = framework.GetBundleContext();
        auto sref = context.RegisterService<int1>(std::make_shared<TestServiceA>(100)).GetReference();
        auto sref2 = context.RegisterService<int1>(std::make_shared<TestServiceA>(85)).GetReference();

        auto service1 = context.GetService(sref);
        ASSERT_EQ(service1->getValue(), 100);
        auto service2 = context.GetService(sref2);
        ASSERT_EQ(service2->getValue(), 85);
        ASSERT_EQ(cppmicroservices::GetServiceReference(service1), sref);
        ASSERT_FALSE(cppmicroservices::GetServiceReference(service1) == sref2);
        ASSERT_EQ(cppmicroservices::GetServiceReference(service2), sref2);

        // Install and start bundle
        auto testBundle = ::test::InstallAndStartBundle(ctx, "TestBundleDSCA05");
        ASSERT_TRUE(testBundle);

        auto dsRef = context.GetServiceReference<test::CAInterface>();
        ASSERT_TRUE(dsRef);
        auto dsSvc = context.GetService<test::CAInterface>(dsRef);
        auto retSRef = cppmicroservices::GetServiceReference(dsSvc);
        ASSERT_EQ(retSRef, dsRef);
        ASSERT_EQ(cppmicroservices::any_cast<std::string>(retSRef.GetProperty("ManifestProp")), "abc");
    }
} // namespace test
