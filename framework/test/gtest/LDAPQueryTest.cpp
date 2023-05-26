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

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/LDAPProp.h"

#include "TestUtils.h"
#include "cppmicroservices/LDAPFilter.h"
#include "gtest/gtest.h"

using namespace cppmicroservices;

class LDAPQueryTest : public ::testing::Test
{
  protected:
    Bundle testBundle;
    Framework f;

  public:
    LDAPQueryTest() : f(FrameworkFactory().NewFramework()) {};

    ~LDAPQueryTest() override = default;

    void
    SetUp() override
    {
        f.Start();
        testBundle = cppmicroservices::testing::InstallLib(f.GetBundleContext(), "TestBundleLQ");
    }

    void
    TearDown() override
    {
        f.Stop();
        f.WaitForStop(std::chrono::milliseconds::zero());
    }
};

TEST_F(LDAPQueryTest, TestLDAPFilterMatchBundle)
{
    LDAPFilter ldapMatchCase("(bundle.testproperty=YES)");
    LDAPFilter ldapKeyMismatchCase("(bundle.TestProperty=YES)");
    LDAPFilter ldapValueMismatchCase("(bundle.testproperty=Yes)");

    // Exact string match of both key and value
    ASSERT_TRUE(ldapMatchCase.Match(testBundle));

    // Testing case-insensitive key (should still pass)
    ASSERT_TRUE(ldapKeyMismatchCase.Match(testBundle));

    // Testing case-insensitive value (should fail)
    ASSERT_FALSE(ldapValueMismatchCase.Match(testBundle));
}

TEST_F(LDAPQueryTest, TestLDAPFilterMatchNoException)
{
    LDAPFilter ldapMatch("(hosed=1)");
    AnyMap props(AnyMap::UNORDERED_MAP);
    props["hosed"] = std::string("1");
    props["hosedd"] = std::string("yum");
    props["hose"] = std::string("yum");

    // Testing no exception is thrown.
    ASSERT_NO_THROW(ldapMatch.Match(props));

    // Testing key match
    ASSERT_TRUE(ldapMatch.Match(props));

    // Testing no exception is thrown.
    ASSERT_NO_THROW(ldapMatch.Match(testBundle));

    // Testing key match
    ASSERT_TRUE(ldapMatch.Match(testBundle));

    AnyMap props1(AnyMap::UNORDERED_MAP);
    props1["hosed"] = std::string("1");
    props1["HOSED"] = std::string("yum");

    // Testing exception for case variants of the same key.
    ASSERT_THROW(ldapMatch.Match(props1), std::runtime_error);
}

TEST_F(LDAPQueryTest, TestLDAPFilterMatchServiceReferenceBase)
{
    LDAPFilter ldapMatchCase("(service.testproperty=YES)");
    LDAPFilter ldapKeyMismatchCase("(service.TestProperty=YES)");
    LDAPFilter ldapValueMismatchCase("(service.testproperty=Yes)");

    testBundle.Start();

    auto thisBundleCtx = testBundle.GetBundleContext();
    ServiceReferenceU sr = thisBundleCtx.GetServiceReference("cppmicroservices::TestBundleLQService");

    // Make sure the obtained ServiceReferenceBase object is not null
    ASSERT_TRUE(sr);

    // Exact string match of both key and value
    ASSERT_TRUE(ldapMatchCase.Match(sr));

    // Testing case-insensitive key (should still pass)
    ASSERT_TRUE(ldapKeyMismatchCase.Match(sr));

    // Testing case-insensitive value (should fail)
    ASSERT_FALSE(ldapValueMismatchCase.Match(sr));

    testBundle.Stop();

    // Testing the behavior after the bundle has stopped (service properties
    // should still be available for queries according to OSGi spec 5.2.1).
    ASSERT_TRUE(ldapMatchCase.Match(sr));
}

#ifdef SUPPORT_NESTED_LOOKUP
TEST_F(LDAPQueryTest, TestNestedData)
{
    LDAPFilter filter1(LDAPProp("a.b.c.d") == 5);
    LDAPFilter filter2(LDAPProp("a.e.f.g") == 6);
    LDAPFilter filter3(LDAPProp("h.i.j.k") == 12);
    LDAPFilter filter4(LDAPProp("h.i.l.m.n") == true);
    LDAPFilter filter5(LDAPProp("bundle.testproperty") == "YES");
    LDAPFilter filter6(LDAPProp("bundle.nestedproperty.foo") == "bar");
    LDAPFilter filter7(LDAPProp("i.expect.this.to.fail") == true);

    auto const& headers = testBundle.GetHeaders();

    /*
        a: {
            b: 1
        }
    */
    cppmicroservices::AnyMap uomTestMap(cppmicroservices::AnyMap::UNORDERED_MAP);
    cppmicroservices::AnyMap uomTestMapNested(cppmicroservices::AnyMap::UNORDERED_MAP);
    uomTestMapNested["b"] = 1;
    uomTestMap["a"] = uomTestMapNested;

    LDAPFilter filter8(LDAPProp("a.b") == 1);
    LDAPFilter filter9(LDAPProp("a.B") == 1);

    cppmicroservices::AnyMap uociTestMap(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    uociTestMap["A"] = 1;
    LDAPFilter filter10(LDAPProp("a") == 1);

    /*
        a: {
            b: 1
        }
    */
    cppmicroservices::AnyMap omTestMap(cppmicroservices::AnyMap::ORDERED_MAP);
    cppmicroservices::AnyMap omTestMapNested(cppmicroservices::AnyMap::ORDERED_MAP);
    omTestMapNested["b"] = 1;
    omTestMap["a"] = omTestMapNested;
    LDAPFilter filter11(LDAPProp("a.B") == 1);

    ASSERT_TRUE(filter1.Match(headers));
    ASSERT_TRUE(filter2.Match(headers));
    ASSERT_TRUE(filter3.Match(headers));
    ASSERT_TRUE(filter4.Match(headers));
    ASSERT_TRUE(filter5.Match(headers));
    ASSERT_TRUE(filter6.Match(headers));
    ASSERT_FALSE(filter7.Match(headers));

    // UOM
    ASSERT_TRUE(filter8.Match(uomTestMap));
    ASSERT_FALSE(filter9.MatchCase(uomTestMap));
    ASSERT_TRUE(filter9.Match(uomTestMap));

    // UOCI
    ASSERT_TRUE(filter10.Match(uociTestMap));
    ASSERT_FALSE(filter10.MatchCase(uociTestMap));

    // OM
    ASSERT_TRUE(filter11.Match(omTestMap));
    ASSERT_FALSE(filter11.MatchCase(omTestMap));
}
#endif
