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
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/JSONFilter.h"
#include "cppmicroservices/ServiceReference.h"

#include "gtest/gtest.h"

using namespace cppmicroservices;

TEST(JSONFilter, ToString)
{
  JSONFilter filter;
  ASSERT_NO_THROW(filter.ToString());
  ASSERT_NO_THROW(std::cout << "Empty JSONFilter: " << filter << std::endl;);
}

TEST(JSONFilter, BooleanOperator)
{
  JSONFilter emptyFilter;
  ASSERT_FALSE(emptyFilter);

  JSONFilter validFilter("cn == 'Babs Jensen'");
  ASSERT_TRUE(validFilter);
}

TEST(JSONFilter, Comparison)
{
  JSONFilter filter1, filter2;
  ASSERT_TRUE(filter1 == filter2);

  JSONFilter filt("cn == 'Babs Jensen'");
  ASSERT_FALSE(filter1 == filt);
}

TEST(JSONFilter, Equality)
{
  JSONFilter jsonfilt1("prod == 'CppMiroServices'");
  JSONFilter jsonfilt2("prod == 'CppMiroServices'");
  ASSERT_EQ(jsonfilt1, jsonfilt2);

  JSONFilter jsonfilt3("prod == microservices");
  ASSERT_FALSE( jsonfilt1 == jsonfilt3);

}

TEST(JSONFilter, TestParsing)
{
  // WELL FORMED Expr
   EXPECT_NO_THROW(JSONFilter json("cn=='Babs Jensen'"));
 
  // MALFORMED Expr
  EXPECT_THROW(JSONFilter json("cn='Babs *"), std::invalid_argument);
}
TEST(JSONFilter, DefaultConstructedMatch)
{
  JSONFilter filter;
  ASSERT_NO_THROW(filter.Match(AnyMap(any_map::map_type::ORDERED_MAP)));
  ASSERT_NO_THROW(filter.Match(ServiceReferenceU()));
  ASSERT_NO_THROW(filter.Match(Bundle()));
}

TEST(JSONFilter,DotNames)
{
  AnyMap serviceIdMap(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  serviceIdMap["service.id"] = 1;

  
  JSONFilter filter("\"service.id\">=`0`");
  bool rval = filter.Match(serviceIdMap);
  ASSERT_TRUE(rval);
}
