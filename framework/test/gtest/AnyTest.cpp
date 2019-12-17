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

#include "cppmicroservices/Any.h"
#include "cppmicroservices/GlobalConfig.h"

#include "gtest/gtest.h"

using namespace cppmicroservices;

template<typename T>
void TestUnsafeAnyCast(Any& anyObj, T val)
{
  T* valPtr = unsafe_any_cast<T>(&anyObj);
  EXPECT_NE(valPtr, nullptr);
  EXPECT_EQ(*valPtr, val);
}

TEST(AnyTest, EmptyAny)
{
  Any anyEmpty;
  EXPECT_THROW(anyEmpty.ToString(), std::logic_error);
  EXPECT_NO_THROW(anyEmpty.ToStringNoExcept());
}

TEST(AnyTest, AnyBool)
{
  Any anyBool = true;
  EXPECT_EQ(anyBool.Type(), typeid(bool));
  EXPECT_EQ(any_cast<bool>(anyBool), true);
  EXPECT_EQ(anyBool.ToString(), "1");
  EXPECT_EQ(anyBool.ToJSON(), "true");
  TestUnsafeAnyCast<bool>(anyBool, true);

  anyBool = false;
  EXPECT_EQ(anyBool.ToString(), "0");
  EXPECT_EQ(anyBool.ToJSON(), "false");
  TestUnsafeAnyCast(anyBool, false);
}

TEST(AnyTest, AnyInt)
{
  Any anyInt = 13;
  EXPECT_EQ(anyInt.Type(), typeid(int));
  EXPECT_EQ(any_cast<int>(anyInt), 13);
  EXPECT_EQ(anyInt.ToString(), "13");
  EXPECT_EQ(anyInt.ToJSON(), "13");
  TestUnsafeAnyCast<int>(anyInt, 13);
}

TEST(AnyTest, AnyChar)
{
  Any anyChar = 'a';
  EXPECT_EQ(anyChar.Type(), typeid(char));
  EXPECT_EQ(any_cast<char>(anyChar), 'a');
  EXPECT_EQ(anyChar.ToString(), "a");
  EXPECT_EQ(anyChar.ToJSON(), "a");
  TestUnsafeAnyCast<char>(anyChar, 'a');
}

TEST(AnyTest, AnyFloat)
{
  Any anyFloat = 0.2f;
  EXPECT_EQ(anyFloat.Type(), typeid(float));
  EXPECT_LT(any_cast<float>(anyFloat) - 0.2f,
            std::numeric_limits<float>::epsilon());
  EXPECT_EQ(anyFloat.ToString(), "0.2");
  EXPECT_EQ(anyFloat.ToJSON(), "0.2");
  TestUnsafeAnyCast<float>(anyFloat, 0.2f);
}

TEST(AnyTest, AnyDouble)
{
  Any anyDouble = 0.5;
  EXPECT_EQ(anyDouble.Type(), typeid(double));
  EXPECT_LT(any_cast<double>(anyDouble) - 0.5,
            std::numeric_limits<double>::epsilon());
  EXPECT_EQ(anyDouble.ToString(), "0.5");
  EXPECT_EQ(anyDouble.ToJSON(), "0.5");
  TestUnsafeAnyCast<double>(anyDouble, 0.5);
}

TEST(AnyTest, AnyString)
{
  Any anyString = std::string("bonjour");
  EXPECT_EQ(anyString.Type(), typeid(std::string));
  EXPECT_EQ(any_cast<std::string>(anyString), "bonjour");
  EXPECT_EQ(anyString.ToString(), "bonjour");
  EXPECT_EQ(anyString.ToJSON(), "\"bonjour\"");
  TestUnsafeAnyCast<std::string>(anyString, std::string("bonjour"));
}

TEST(AnyTest, AnyVector)
{
  std::vector<int32_t> vecInts = { 1, 2 };
  Any anyVectorOfInts = vecInts;
  EXPECT_EQ(anyVectorOfInts.Type(), typeid(std::vector<int32_t>));
  EXPECT_EQ(any_cast<std::vector<int32_t>>(anyVectorOfInts), vecInts);
  EXPECT_EQ(anyVectorOfInts.ToString(), "[1,2]");
  EXPECT_EQ(anyVectorOfInts.ToJSON(), "[1,2]");
}

TEST(AnyTest, AnyList)
{
  std::list<int32_t> listInts = { 1, 2 };
  Any anyListOfInts = listInts;
  EXPECT_EQ(anyListOfInts.Type(), typeid(std::list<int32_t>));
  EXPECT_EQ(any_cast<std::list<int32_t>>(anyListOfInts), listInts);
  EXPECT_EQ(anyListOfInts.ToString(), "[1,2]");
  EXPECT_EQ(anyListOfInts.ToJSON(), "[1,2]");
}

TEST(AnyTest, AnySet)
{
  std::set<int32_t> setInts = { 1, 2 };
  Any anySetOfInts = setInts;
  EXPECT_EQ(anySetOfInts.Type(), typeid(std::set<int32_t>));
  EXPECT_EQ(any_cast<std::set<int32_t>>(anySetOfInts), setInts);
  EXPECT_EQ(anySetOfInts.ToString(), "[1,2]");
  EXPECT_EQ(anySetOfInts.ToJSON(), "[1,2]");
}

TEST(AnyTest, AnyVectorAny)
{
  std::vector<Any> vecAny = { 1, std::string("bonjour") };
  Any anyVectorOfAnys = vecAny;
  EXPECT_EQ(anyVectorOfAnys.Type(), typeid(std::vector<Any>));
  EXPECT_EQ(anyVectorOfAnys.ToString(), "[1,bonjour]");
  EXPECT_EQ(anyVectorOfAnys.ToJSON(), "[1,\"bonjour\"]");
}

TEST(AnyTest, AnyListAny)
{
  std::list<Any> listAny = { 1, std::string("bonjour") };
  Any anyListOfAnys = listAny;
  EXPECT_EQ(anyListOfAnys.Type(), typeid(std::list<Any>));
  EXPECT_EQ(anyListOfAnys.ToString(), "[1,bonjour]");
  EXPECT_EQ(anyListOfAnys.ToJSON(), "[1,\"bonjour\"]");
}

TEST(AnyTest, AnyMap)
{
  std::map<std::string, int32_t> map = { { std::string("one"), 1 },
                                         { std::string("two"), 2 } };
  Any anyMap = map;
  EXPECT_EQ(anyMap.Type(), typeid(std::map<std::string, int32_t>));
  EXPECT_EQ((any_cast<std::map<std::string, int32_t>>(anyMap)), map);
  EXPECT_EQ(anyMap.ToString(), "{one : 1, two : 2}");
  EXPECT_EQ(anyMap.ToJSON(), "{\"one\" : 1, \"two\" : 2}");
}

TEST(AnyTest, AnyMapAny)
{
  std::map<int32_t, Any> map = { { 1, 0.3 }, { 3, std::string("bonjour") } };
  Any anyMap = map;
  EXPECT_EQ(anyMap.Type(), typeid(std::map<int32_t, Any>));
  EXPECT_EQ(anyMap.ToString(), "{1 : 0.3, 3 : bonjour}");
  EXPECT_EQ(anyMap.ToJSON(), "{\"1\" : 0.3, \"3\" : \"bonjour\"}");
}

TEST(AnyTest, AnyMapComplex)
{
  std::map<int32_t, Any> mapToInsert = { { 1, 0.3 },
                                         { 3, std::string("bonjour") } };
  std::vector<int32_t> numbers = { 9, 8, 7 };
  std::map<std::string, Any> map = { { "number", 5 },
                                     { "vector", numbers },
                                     { "map", mapToInsert } };

  std::string toStringRes =
    "{map : {1 : 0.3, 3 : bonjour}, number : 5, vector : [9,8,7]}";
  std::string toJSONRes = "{\"map\" : {\"1\" : 0.3, \"3\" : \"bonjour\"}, "
                          "\"number\" : 5, \"vector\" : [9,8,7]}";

  Any anyMap = map;
  EXPECT_EQ(anyMap.Type(), typeid(std::map<std::string, Any>));
  EXPECT_EQ(anyMap.ToString(), toStringRes);
  EXPECT_EQ(anyMap.ToJSON(), toJSONRes);
}

TEST(AnyTest, AnyBadAnyCastException) {
  const Any uncastableConstAny(0.0);
  Any uncastableAny(0.0);

  EXPECT_THROW(any_cast<std::string>(uncastableConstAny), cppmicroservices::BadAnyCastException);
  EXPECT_THROW(any_cast<std::string>(uncastableAny),
               cppmicroservices::BadAnyCastException);
  EXPECT_THROW(ref_any_cast<std::string>(uncastableConstAny),
               cppmicroservices::BadAnyCastException);
  EXPECT_THROW(ref_any_cast<std::string>(uncastableAny),
               cppmicroservices::BadAnyCastException);
}