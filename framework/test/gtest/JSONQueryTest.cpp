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

#include "TestUtils.h"
#include "cppmicroservices/JSONFilter.h"
#include "gtest/gtest.h"

using namespace cppmicroservices;

class JSONQueryTest : public ::testing::Test
{
protected:
  Bundle testBundle;
  Framework f;

public:
  JSONQueryTest()
    : f(FrameworkFactory().NewFramework()){};

  ~JSONQueryTest() override = default;

  void SetUp() override
  {
    f.Start();
    testBundle = cppmicroservices::testing::InstallLib(f.GetBundleContext(),
                                                       "TestBundleLQ");
  }

  void TearDown() override
  {
    f.Stop();
    f.WaitForStop(std::chrono::milliseconds::zero());
  }
};

TEST_F(JSONQueryTest, TestJSONFilterMatchBundle)
{
  
  JSONFilter json_match("\"bundle.nestedproperty\".foo=='bar'");
  JSONFilter json_nomatch("\"bundle.nestedproperty\".foo=='Bar'");

  // Exact string match of both key and value
  ASSERT_TRUE(json_match.Match(testBundle));

  // Testing case-insensitive value (should fail)
  ASSERT_FALSE(json_nomatch.Match(testBundle));
 
}

TEST_F(JSONQueryTest, TestJSONFilterMatchServiceReferenceBase)
{
  // non-nested property
  JSONFilter json_match("\"service.testproperty\"=='YES'");

  // non-nested property, expecting a no match due to case-sensitivity of JSON queries
  JSONFilter json_nomatch("\"service.TestProperty\"=='YES'");

  // nested property
  JSONFilter json_nested("\"service.nestedproperty\".foo=='bar'");

  testBundle.Start();

  auto thisBundleCtx = testBundle.GetBundleContext();
  ServiceReferenceU sr =
    thisBundleCtx.GetServiceReference("cppmicroservices::TestBundleLQService");

  // Make sure the obtained ServiceReferenceBase object is not null
  ASSERT_TRUE(sr);

  // Expected match
  ASSERT_TRUE(json_match.Match(sr));

  // Expected no match
  ASSERT_FALSE(json_nomatch.Match(sr));

  // Testing nested property matching
  ASSERT_TRUE(json_nested.Match(sr));

  testBundle.Stop();

  // Testing the behavior after the bundle has stopped (service properties
  // should still be available for queries according to OSGi spec 5.2.1).
  ASSERT_TRUE(json_match.Match(sr));
}

TEST_F(JSONQueryTest, TestJSONFilterMatchNoException)
{
  JSONFilter jsonMatch("\"hosed\"=='1'");
  AnyMap props(AnyMap::UNORDERED_MAP);
  props["hosed"] = std::string("1");
  props["hosedd"] = std::string("yum");
  props["hose"] = std::string("yum");

  // Testing no exception is thrown.
  ASSERT_NO_THROW(jsonMatch.Match(props));

  // Testing key match
  ASSERT_TRUE(jsonMatch.Match(props));

  // Testing no exception is thrown.
  ASSERT_NO_THROW(jsonMatch.Match(testBundle));

  // Testing key match
  ASSERT_TRUE(jsonMatch.Match(testBundle));

  AnyMap props1(AnyMap::UNORDERED_MAP);
  props1["hosed"] = std::string("1");
  props1["HOSED"] = std::string("yum");

  // Testing exception for case variants of the same key.
  ASSERT_THROW(jsonMatch.Match(props1), std::runtime_error);
}

TEST_F(JSONQueryTest, TestJSONFilterMatchAnyMap)
{
  // Make sure Match's key look-up is case-sensitive
  JSONFilter json("cn == 'Babs Jensen'");
  AnyMap props(AnyMap::UNORDERED_MAP);

  // Several values
  props["cn"] = std::string("Babs Jensen");
  props["unused"] = std::string("Jansen");
  ASSERT_TRUE(json.Match(props));

 
  // NOT FOUND
  json = JSONFilter("cn =='Babs Jensen'");
  props.clear();
  props["unused"] = std::string("New");
  ASSERT_FALSE(json.Match(props));

  // std::vector with integer values
  json = JSONFilter("sn[?@ == '1'] | [0] == '1'");
  props.clear();
  std::vector<Any> list;
  list.push_back(std::string("1"));
  list.push_back(std::string("Babs Jensen"));
  props["sn"] = list;
  ASSERT_TRUE(json.Match(props));

  //nested
  /* Create a AnyMap with the following JSON representation:
   *
   *   uoci : {
   *		vec : { 
   *			First : 1,
   *			Second : {
							hi : "hi"
							there : "there" 
						 }
   *        }
   *   }
   */
  std::string filter_str = "vec.Second.there =='there'";
  json = JSONFilter(filter_str);

  AnyMap uo(AnyMap::UNORDERED_MAP);
  uo["hi"] = std::string("hi");
  uo["there"] = std::string("there");

  AnyMap uoc(AnyMap::UNORDERED_MAP);
  uoc["First"] = 1;
  uoc["Second"] = uo;

  AnyMap uoci(AnyMap::UNORDERED_MAP);

  uoci["vec"] = uoc;
  ASSERT_TRUE(json.Match(uoci));
}




