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

#include "cppmicroservices/LDAPFilter.h"
#include "cppmicroservices/LDAPProp.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceEvent.h"
#include "cppmicroservices/ServiceTracker.h"

#include "gtest/gtest.h"

using namespace cppmicroservices;

TEST(LDAPTest, LDAPFilterBoolEquals)
{
  LDAPFilter ldap("(prod=CppMiroServices)");
  LDAPFilter ldap_alt("(prod=CppMiroServices)");
  ASSERT_TRUE(ldap);
  ASSERT_EQ(ldap, ldap_alt);
}

TEST(LDAPTest, LDAPProp)
{
  Any any1 = std::string("hello");
  Any any2 = std::string("bye");
  Any any3 = std::string("Ballpark");
  Any anyInt1 = 30;
  Any anyInt2 = 50;
  LDAPFilter filter(
    LDAPProp("bla") != "jo" && LDAPProp("foo") == any1 && LDAPProp("bar") != any2 &&
    LDAPProp("baz") >= anyInt1 && LDAPProp("bleh") <= anyInt2 && LDAPProp("doh").Approx(any3)
  );
  const std::string filterStr = "(&(&(&(&(&(!(bla=jo))(foo=hello))(!(bar=bye)))(baz>=30))(bleh<=50))(doh~=Ballpark))";
  ASSERT_EQ(filter.ToString(), filterStr);
}

TEST(LDAPTest, LDAPExprGetMatchedObjectClasses)
{
  struct MyInterfaceOne {
    virtual ~MyInterfaceOne() {}
  };
  struct MyServiceOne : public MyInterfaceOne {};

  auto f = FrameworkFactory().NewFramework();
  f.Init();
  BundleContext context{ f.GetBundleContext() };

  auto serviceOne = std::make_shared<MyServiceOne>();
  context.RegisterService<MyInterfaceOne>(serviceOne);

  LDAPFilter filter("(&(objectclass=alpha)(objectclass=beta))");
  ServiceTracker<MyInterfaceOne> tracker(context, filter, nullptr);
  tracker.Open();

  LDAPFilter filter1("(&(objectclass=beta)(objectclass=alpha))");
  ServiceTracker<MyInterfaceOne> tracker1(context, filter1, nullptr);
  tracker1.Open();

  LDAPFilter filter2("(|(objectclass=alpha)(objectclass=beta))");
  ServiceTracker<MyInterfaceOne> tracker2(context, filter2, nullptr);
  tracker2.Open();

  ASSERT_TRUE(tracker.GetServiceReferences().size() == 0);
}

TEST(LDAPTest, LDAPExprIsSimple)
{
  auto f = FrameworkFactory().NewFramework();
  f.Init();
  BundleContext fCtx{ f.GetBundleContext() };

  auto lambda = [](const ServiceEvent&)
  {
    std::cout << "ServiceEvent!";
  };

  const std::string ldapFilter = "(|(objectClass=foo)(objectClass=bar))";
  ASSERT_TRUE(fCtx.AddServiceListener(lambda, ldapFilter));
  const std::string ldapFilter2 = "(|(&(objectClass=foo)(objectClass=bar))(objectClass=baz))";
  ASSERT_TRUE(fCtx.AddServiceListener(lambda, ldapFilter2));
}

TEST(LDAPTest, LDAPExprEvaluateNot)
{
  LDAPFilter ldapMatch("(!(hosed=1))");
  AnyMap props(AnyMap::UNORDERED_MAP);
  props["hosed"] = std::string("2");
  ASSERT_TRUE(ldapMatch.Match(props));
}

TEST(LDAPTest, LDAPExprCompare)
{
  // Testing wildcard
  LDAPFilter ldapMatch("(hosed=*)");
  AnyMap props(AnyMap::UNORDERED_MAP);
  props["hosed"] = 5;
  ASSERT_TRUE(ldapMatch.Match(props));

  // Testing empty
  ldapMatch = LDAPFilter("(hosed=1)");
  props.clear();
  props["hosed"] = Any();
  ASSERT_FALSE(ldapMatch.Match(props));

  // Testing AnyMap value types.
  props.clear();
  props["hosed"] = std::list<std::string>{ "1", "2" };
  ASSERT_TRUE(ldapMatch.Match(props));

  props.clear();
  props["hosed"] = '1';
  ASSERT_TRUE(ldapMatch.Match(props));

  ldapMatch = LDAPFilter("(val<=true)");
  props.clear();
  props["val"] = true;
  ASSERT_FALSE(ldapMatch.Match(props));

  // Testing integral types.
  ldapMatch = LDAPFilter("(hosed=1)");
  props.clear();
  props["hosed"] = static_cast<short>(1);
  ASSERT_TRUE(ldapMatch.Match(props));
  props.clear();
  props["hosed"] = static_cast<long long int>(1);
  ASSERT_TRUE(ldapMatch.Match(props));
  props.clear();
  props["hosed"] = static_cast<unsigned char>(1);
  ASSERT_TRUE(ldapMatch.Match(props));
  props.clear();
  props["hosed"] = static_cast<unsigned short>(1);
  ASSERT_TRUE(ldapMatch.Match(props));
  props.clear();
  props["hosed"] = static_cast<unsigned int>(1);
  ASSERT_TRUE(ldapMatch.Match(props));
  props.clear();
  props["hosed"] = static_cast<unsigned long int>(1);
  ASSERT_TRUE(ldapMatch.Match(props));
  ldapMatch = LDAPFilter("(hosed<=200)");
  props.clear();
  props["hosed"] = 1;
  ASSERT_TRUE(ldapMatch.Match(props));
  // Test integer overflow
  ldapMatch = LDAPFilter("(hosed=LONG_MAX)");
  props.clear();
  props["hosed"] = 1;
  ASSERT_FALSE(ldapMatch.Match(props));

  // Testing floating point types.
  ldapMatch = LDAPFilter("(hosed=1)");
  props.clear();
  props["hosed"] = static_cast<float>(1.0);
  ASSERT_TRUE(ldapMatch.Match(props));
  // Test floating point overflow
  ldapMatch = LDAPFilter("(hosed=1.18973e+4932zzz)");
  ASSERT_FALSE(ldapMatch.Match(props));
  ldapMatch = LDAPFilter("(hosed>=0.1)");
  ASSERT_TRUE(ldapMatch.Match(props));
  ldapMatch = LDAPFilter("(hosed<=2.0)");
  ASSERT_TRUE(ldapMatch.Match(props));

  props.clear();
  props["hosed"] = static_cast<double>(1.0);
  ldapMatch = LDAPFilter("(hosed=1)");
  ASSERT_TRUE(ldapMatch.Match(props));
  // Test floating point overflow
  ldapMatch = LDAPFilter("(hosed=1.18973e+4932zzz)");
  ASSERT_FALSE(ldapMatch.Match(props));
  ldapMatch = LDAPFilter("(hosed>=0.1)");
  ASSERT_TRUE(ldapMatch.Match(props));
  ldapMatch = LDAPFilter("(hosed<=2.0)");
  ASSERT_TRUE(ldapMatch.Match(props));
}

TEST(LDAPTest, LDAPExprCompareString)
{
  LDAPFilter ldapMatch("(name>=abra)");
  AnyMap props(AnyMap::UNORDERED_MAP);
  props["name"] = std::string("cadabra");
  ASSERT_TRUE(ldapMatch.Match(props));

  ldapMatch = LDAPFilter("(name<=oink)");
  ASSERT_TRUE(ldapMatch.Match(props));

  ldapMatch = LDAPFilter("(name~=micro)");
  props.clear();
  props["name"] = std::string("MICRO");
  ASSERT_TRUE(ldapMatch.Match(props));
}

TEST(LDAPTest, LDAPExprPatSubstr)
{
  LDAPFilter ldapMatch("(name=ab*d)");
  AnyMap props(AnyMap::UNORDERED_MAP);
  props["name"] = std::string("ab");
  ASSERT_FALSE(ldapMatch.Match(props));

  ldapMatch = LDAPFilter("(name=abcd)");
  props.clear();
  props["name"] = std::string("ab");
  ASSERT_FALSE(ldapMatch.Match(props));
}

TEST(LDAPTest, LDAPExprParseExceptions)
{
  // The numbers in the comments mean the error in the
  // lines of LDAPExpr.cpp. These will be amended in a future checkin
  // Testing #173
  EXPECT_THROW(LDAPFilter("(name=abra)zxdzx"), std::invalid_argument);
  // Testing #653
  EXPECT_THROW(LDAPFilter("(!(name=abra)(name=beta))"), std::invalid_argument);
  // Testing #662 & #811
  EXPECT_THROW(LDAPFilter("(=abra)"), std::invalid_argument);
  // Testing #675
  EXPECT_THROW(LDAPFilter("(name>abra)"), std::invalid_argument);
  // Testing #679
  EXPECT_THROW(LDAPFilter("(name=abra("), std::invalid_argument);
  // Why #711?
  // Testing #748
  EXPECT_THROW(LDAPFilter(""), std::invalid_argument);
  // Testing #767
  EXPECT_THROW(LDAPFilter("(name=abra"), std::invalid_argument);
  // Testing #832
  ASSERT_EQ(LDAPFilter("(name=ab\\a)"), LDAPFilter("(name=aba)"));
}

