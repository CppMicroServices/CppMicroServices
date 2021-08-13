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
  /*
  JSONFilter json_match("\"bundle.nestedproperty\".foo=='bar'");
  JSONFilter json_nomatch("\"bundle.nestedproperty\".foo=='Bar'");

  // Exact string match of both key and value
  ASSERT_TRUE(json_match.Match(testBundle));

  // Testing case-insensitive value (should fail)
  ASSERT_FALSE(json_nomatch.Match(testBundle));
   */
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
