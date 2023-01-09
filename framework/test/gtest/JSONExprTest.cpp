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
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/FilterAdapter.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/JSONFilter.h"
#include "cppmicroservices/JSONProp.h"
#include "cppmicroservices/ServiceEvent.h"
#include "cppmicroservices/ServiceTracker.h"

#include "gtest/gtest.h"

using namespace cppmicroservices;

/*TEST(JSONExprTest, GetMatchedObjectClasses)
{
  // Improve coverage for LDAPExpr::GetMatchedObjectClasses()
  struct MyInterfaceOne
  {
    virtual ~MyInterfaceOne() {}
  };
  struct MyServiceOne : public MyInterfaceOne
  {};

  auto f = FrameworkFactory().NewFramework();
  f.Init();
  BundleContext context{ f.GetBundleContext() };

  auto serviceOne = std::make_shared<MyServiceOne>();
  context.RegisterService<MyInterfaceOne>(serviceOne);

  JSONFilter filter("(&(objectclass=alpha)(objectclass=beta))");
  ServiceTracker<MyInterfaceOne> tracker(context, filter, nullptr);
  tracker.Open();

  JSONFilter filter1("(&(objectclass=beta)(objectclass=alpha))");
  ServiceTracker<MyInterfaceOne> tracker1(context, filter1, nullptr);
  tracker1.Open();

  JSONFilter filter2("(|(objectclass=alpha)(objectclass=beta))");
  ServiceTracker<MyInterfaceOne> tracker2(context, filter2, nullptr);
  tracker2.Open();

  JSONFilter filter3("(&(objectclass=alpha)(objectclass=alpha))");
  ServiceTracker<MyInterfaceOne> tracker3(context, filter3, nullptr);
  tracker3.Open();

  JSONFilter filter4("(|(object=alpha)(object=beta))");
  ServiceTracker<MyInterfaceOne> tracker4(context, filter4, nullptr);
  tracker4.Open();

  ASSERT_TRUE(tracker.GetServiceReferences().size() == 0);
}

TEST(JSONExprTest, IsSimple)
{
  // Expanding coverage for LDAPExpr::IsSimple by testing the OR filter.
  auto f = FrameworkFactory().NewFramework();
  f.Init();
  BundleContext fCtx{ f.GetBundleContext() };

  auto lambda = [](const ServiceEvent&) { std::cout << "ServiceEvent!"; };

  const std::string jsonFilter = "(|(objectClass=foo)(objectClass=bar))";
  ASSERT_TRUE(fCtx.AddServiceListener(lambda, jsonFilter));
  const std::string jsonFilter2 =
    "(|(&(objectClass=foo)(objectClass=bar))(objectClass=baz))";
  ASSERT_TRUE(fCtx.AddServiceListener(lambda, jsonFilter2));
}
*/
TEST(JSONExprTest, Evaluate)
{
    // Testing previously uncovered lines in LDAPExpr::Evaluate()
    // case NOT
    JSONFilter jsonMatch(std::string("\"hosted\" != '1'"));
    AnyMap props(AnyMap::UNORDERED_MAP);
    props["hosed"] = std::string("2");
    ASSERT_TRUE(jsonMatch.Match(props));

    // case OR returning false
    JSONFilter jsonMatch2("\"hosed\"=='2' || \"hosed\"=='5'");
    props["hosed"] = std::string("3");
    ASSERT_FALSE(jsonMatch2.Match(props));
}

TEST(JSONExprTest, Compare)
{
    // Testing wildcard
    // TODO: chheck if '*' wildcard is required
    // what is use of it in case of ldap filters ?
    JSONFilter jsonMatch("\"hosed\"=='*'");
    AnyMap props(AnyMap::UNORDERED_MAP);
    props["hosed"] = 5;
    // ASSERT_TRUE(jsonMatch.Match(props));

    // Testing empty
    jsonMatch = JSONFilter("\"hosed\"=='1'");
    props.clear();
    props["hosed"] = Any();
    ASSERT_FALSE(jsonMatch.Match(props));

    // Testing AnyMap value types.
    // TODO: Implement contains method for json prop
    props.clear();
    props["hosed"] = std::list<std::string> { "1", "2" };
    ASSERT_FALSE(jsonMatch.Match(props));

    props.clear();
    props["hosed"] = '1';
    ASSERT_FALSE(jsonMatch.Match(props));

    // Testing integral types.
    jsonMatch = JSONFilter("hosed==`1`");
    props.clear();
    props["hosed"] = static_cast<short>(1);
    ASSERT_TRUE(jsonMatch.Match(props));
    props.clear();
    props["hosed"] = static_cast<long long int>(1);
    ASSERT_TRUE(jsonMatch.Match(props));
    props.clear();
    //  props["hosed"] = static_cast<unsigned char>(1);
    //  ASSERT_TRUE(jsonMatch.Match(props));
    //  props.clear();
    props["hosed"] = static_cast<unsigned short>(1);
    ASSERT_TRUE(jsonMatch.Match(props));
    props.clear();
    props["hosed"] = static_cast<unsigned int>(1);
    ASSERT_TRUE(jsonMatch.Match(props));
    props.clear();
    props["hosed"] = static_cast<unsigned long int>(1);
    ASSERT_TRUE(jsonMatch.Match(props));
    props.clear();
    props["hosed"] = static_cast<unsigned long long int>(1);
    ASSERT_TRUE(jsonMatch.Match(props));
    jsonMatch = JSONFilter("\"hosed\"<=`200`");
    props.clear();
    props["hosed"] = 1;
    ASSERT_TRUE(jsonMatch.Match(props));
    // Test integer overflow
    // TODO: How to deal with LONG_MAX
    jsonMatch = JSONFilter("hosed==LONG_MAX");
    props.clear();
    props["hosed"] = 1;
    ASSERT_FALSE(jsonMatch.Match(props));

    // Testing floating point types.
    jsonMatch = JSONFilter("\"hosed\"==`1`");
    props.clear();
    props["hosed"] = static_cast<float>(1.0);
    ASSERT_TRUE(jsonMatch.Match(props));

    // Test floating point overflow
    // TODO: How to deal with 1.18973e+4932zzz
    /*jsonMatch = JSONFilter("(hosed=1.18973e+4932zzz)");
    ASSERT_FALSE(jsonMatch.Match(props));
    jsonMatch = JSONFilter("(hosed>=0.1)");
    ASSERT_TRUE(jsonMatch.Match(props));
    jsonMatch = JSONFilter("(hosed<=2.0)");
    ASSERT_TRUE(jsonMatch.Match(props));*/

    props.clear();
    props["hosed"] = static_cast<double>(1.0);
    jsonMatch = JSONFilter("\"hosed\"==`1`");
    ASSERT_TRUE(jsonMatch.Match(props));
    // Test floating point overflow
    // TODO: How to deal with 1.18973e+4932zzz
    /*jsonMatch = JSONFilter("(hosed=1.18973e+4932zzz)");
    ASSERT_FALSE(jsonMatch.Match(props));
    jsonMatch = JSONFilter("\"hosed\">=`0.1`)");
    ASSERT_TRUE(jsonMatch.Match(props));
    jsonMatch = JSONFilter("\"hosed\"<=`2.0`");
    ASSERT_TRUE(jsonMatch.Match(props));*/
}

TEST(JSONExprTest, SimplePropUsage)
{
    AnyMap serviceIdMap(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    serviceIdMap["property1"] = std::string("abcd");
    JSONProp prop1("property1");
    JSONProp prop2("property2");

    {
        JSONFilter filter(prop1 == "abcd");
        bool rval = filter.Match(serviceIdMap);
        ASSERT_TRUE(rval);
    }
    {
        serviceIdMap["property2"] = false;
        JSONFilter boolFilter(!prop2);
        bool rval = boolFilter.Match(serviceIdMap);
        ASSERT_TRUE(rval);
    }
    {
        serviceIdMap["property2"] = true;
        JSONFilter boolFilter(!prop2);
        ASSERT_FALSE(boolFilter.Match(serviceIdMap));
    }
    {
        serviceIdMap["property2"] = true;
        JSONFilter boolFilter(prop2 == true);
        ASSERT_TRUE(boolFilter.Match(serviceIdMap));
    }
    {
        serviceIdMap["property2"] = true;
        JSONFilter boolFilter(prop2 == false);
        ASSERT_FALSE(boolFilter.Match(serviceIdMap));
    }
}

TEST(JSONExprTest, ComplexPropUsage)
{
    // from matlab\foundation\secrets_store\api\src\SecretsStore.cpp
    //    ::cppmicroservices::LDAPFilter filter((storage_prop == storage || !storage_prop) &&
    //                                         (implementation_prop == impl || !implementation_prop));

    AnyMap serviceIdMap(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    serviceIdMap["property1"] = std::string("abcd");
    serviceIdMap["property2"] = std::string("efg");
    serviceIdMap["property3"] = false;
    serviceIdMap["property4"] = true;

    JSONProp prop1("property1");
    JSONProp prop2("property2");
    JSONProp prop3("property3");
    JSONProp prop4("property4");

    JSONFilter filter((prop1 == "abcde" || !prop3) && (prop2 == "efgg" || prop4));
    ASSERT_TRUE(filter.Match(serviceIdMap));
}