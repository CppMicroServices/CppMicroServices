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

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/LDAPFilter.h"
#include "cppmicroservices/LDAPProp.h"
#include "cppmicroservices/ServiceProperties.h"

#include "gtest/gtest.h"

#include <string>
#include <vector>

using namespace cppmicroservices;

// --- ServiceRegistry filter cache tests (lines 217, 223) ---

struct ICacheTestService
{
    virtual ~ICacheTestService() {}
};

struct CacheTestServiceImpl : public ICacheTestService
{
};

class ServiceRegistryCacheTest : public ::testing::Test
{
  protected:
    Framework framework;
    BundleContext context;

  public:
    ServiceRegistryCacheTest() : framework(FrameworkFactory().NewFramework()) {}

    void
    SetUp() override
    {
        framework.Start();
        context = framework.GetBundleContext();
    }

    void
    TearDown() override
    {
        framework.Stop();
        framework.WaitForStop(std::chrono::milliseconds::zero());
    }
};

TEST_F(ServiceRegistryCacheTest, CacheHit_SameFilterUsedTwice)
{
    auto svc = std::make_shared<CacheTestServiceImpl>();
    ServiceProperties props;
    props["priority"] = std::string("high");
    auto reg = context.RegisterService<ICacheTestService>(svc, props);

    std::string filter = "(priority=high)";
    auto refs1 = context.GetServiceReferences<ICacheTestService>(filter);
    auto refs2 = context.GetServiceReferences<ICacheTestService>(filter);

    ASSERT_EQ(refs1.size(), 1u);
    ASSERT_EQ(refs2.size(), 1u);
    ASSERT_EQ(refs1[0], refs2[0]);

    reg.Unregister();
}

TEST_F(ServiceRegistryCacheTest, CacheEviction_ManyUniqueFilters)
{
    auto svc = std::make_shared<CacheTestServiceImpl>();
    ServiceProperties props;
    props["idx"] = std::string("target");
    auto reg = context.RegisterService<ICacheTestService>(svc, props);

    for (int i = 0; i < 200; ++i)
    {
        std::string filter = "(key" + std::to_string(i) + "=val)";
        context.GetServiceReferences<ICacheTestService>(filter);
    }

    auto refs = context.GetServiceReferences<ICacheTestService>("(idx=target)");
    ASSERT_EQ(refs.size(), 1u);

    reg.Unregister();
}

// --- LDAPExpr::Evaluate with UNORDERED_MAP_CASEINSENSITIVE_KEYS + matchCase=true (lines 537-546) ---

TEST(LDAPExprCoverage, CaseInsensitiveMap_MatchCase_ExactMatch)
{
    LDAPFilter filter("(Name=hello)");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["Name"] = std::string("hello");

    ASSERT_TRUE(filter.MatchCase(props));
}

TEST(LDAPExprCoverage, CaseInsensitiveMap_MatchCase_CaseMismatchKey)
{
    LDAPFilter filter("(Name=hello)");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["name"] = std::string("hello");

    ASSERT_FALSE(filter.MatchCase(props));
}

TEST(LDAPExprCoverage, CaseInsensitiveMap_MatchCase_KeyNotFound)
{
    LDAPFilter filter("(Name=hello)");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["other"] = std::string("hello");

    ASSERT_FALSE(filter.MatchCase(props));
}

// --- LDAPExpr::Evaluate with ORDERED_MAP (lines 582-622) ---

TEST(LDAPExprCoverage, OrderedMap_CaseInsensitiveMatch)
{
    LDAPFilter filter("(Name=value)");
    AnyMap props(AnyMap::ORDERED_MAP);
    props["name"] = std::string("value");

    ASSERT_TRUE(filter.Match(props));
}

TEST(LDAPExprCoverage, OrderedMap_CaseInsensitiveMatch_NotFound)
{
    LDAPFilter filter("(Name=value)");
    AnyMap props(AnyMap::ORDERED_MAP);
    props["other"] = std::string("value");

    ASSERT_FALSE(filter.Match(props));
}

TEST(LDAPExprCoverage, OrderedMap_CaseSensitiveMatch)
{
    LDAPFilter filter("(Name=value)");
    AnyMap props(AnyMap::ORDERED_MAP);
    props["Name"] = std::string("value");

    ASSERT_TRUE(filter.MatchCase(props));
}

TEST(LDAPExprCoverage, OrderedMap_CaseSensitiveMatch_WrongCase)
{
    LDAPFilter filter("(Name=value)");
    AnyMap props(AnyMap::ORDERED_MAP);
    props["name"] = std::string("value");

    ASSERT_FALSE(filter.MatchCase(props));
}

TEST(LDAPExprCoverage, OrderedMap_MultipleKeys_CaseInsensitive)
{
    LDAPFilter filter("(&(Alpha=1)(Beta=2))");
    AnyMap props(AnyMap::ORDERED_MAP);
    props["alpha"] = std::string("1");
    props["beta"] = std::string("2");

    ASSERT_TRUE(filter.Match(props));
}

TEST(LDAPExprCoverage, OrderedMap_CaseInsensitiveMatch_KeyUpperInMap)
{
    LDAPFilter filter("(Name=value)");
    AnyMap props(AnyMap::ORDERED_MAP);
    props["NAME"] = std::string("value");

    ASSERT_TRUE(filter.Match(props));
}

// --- LDAPExpr::Compare exception catch (lines 810-814) ---

TEST(LDAPExprCoverage, Compare_BadTypeCast_ReturnsFalse)
{
    LDAPFilter filter("(val>=5)");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    // std::vector<Any> with non-comparable types triggers the catch block
    std::vector<Any> list;
    list.push_back(Any());
    props["val"] = list;

    ASSERT_FALSE(filter.Match(props));
}

// --- LDAPExpr::CompareIntegralType non-numeric string (line 826) ---

TEST(LDAPExprCoverage, CompareIntegral_NonNumericFilter_ReturnsFalse)
{
    LDAPFilter filter("(count=notanumber)");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["count"] = 42;

    ASSERT_FALSE(filter.Match(props));
}

TEST(LDAPExprCoverage, CompareIntegral_EmptyString_ReturnsFalse)
{
    LDAPFilter filter("(count=)");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["count"] = 42;

    ASSERT_FALSE(filter.Match(props));
}

// --- LDAPExpr::FixupString via APPROX operator (line 866) ---

TEST(LDAPExprCoverage, ApproxMatch_IgnoresCaseAndWhitespace)
{
    LDAPFilter filter("(name~=Hello World)");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["name"] = std::string("helloworld");

    ASSERT_TRUE(filter.Match(props));
}

TEST(LDAPExprCoverage, ApproxMatch_WithExtraSpaces)
{
    LDAPFilter filter("(name~=FOO BAR)");
    AnyMap props(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    props["name"] = std::string("foo bar");

    ASSERT_TRUE(filter.Match(props));
}

// --- LDAPExpr parse error for malformed filter missing ')' (line 1000) ---

TEST(LDAPExprCoverage, ParseError_MissingClosingParenAfterValue)
{
    EXPECT_THROW(LDAPFilter("(name>=abc"), std::invalid_argument);
}

TEST(LDAPExprCoverage, ParseError_MissingClosingParenApprox)
{
    EXPECT_THROW(LDAPFilter("(name~=abc"), std::invalid_argument);
}

// --- LDAPExpr::ToString with special chars needing escape (line 1033) ---

TEST(LDAPExprCoverage, ToString_EscapesParentheses)
{
    LDAPFilter filter("(name=hello\\(world\\))");
    std::string str = filter.ToString();
    EXPECT_NE(str.find("\\("), std::string::npos);
    EXPECT_NE(str.find("\\)"), std::string::npos);
}

TEST(LDAPExprCoverage, ToString_EscapesBackslash)
{
    LDAPFilter filter("(name=back\\\\slash)");
    std::string str = filter.ToString();
    EXPECT_NE(str.find("\\\\"), std::string::npos);
}

TEST(LDAPExprCoverage, ToString_WildcardRoundTrip)
{
    LDAPFilter filter("(name=hello*world)");
    std::string str = filter.ToString();
    EXPECT_EQ(str, "(name=hello*world)");
}

TEST(LDAPExprCoverage, ToString_ApproxOperator)
{
    LDAPFilter filter("(name~=approx)");
    std::string str = filter.ToString();
    EXPECT_EQ(str, "(name~=approx)");
}

TEST(LDAPExprCoverage, ToString_LEOperator)
{
    LDAPFilter filter("(val<=10)");
    std::string str = filter.ToString();
    EXPECT_EQ(str, "(val<=10)");
}

TEST(LDAPExprCoverage, ToString_GEOperator)
{
    LDAPFilter filter("(val>=10)");
    std::string str = filter.ToString();
    EXPECT_EQ(str, "(val>=10)");
}
