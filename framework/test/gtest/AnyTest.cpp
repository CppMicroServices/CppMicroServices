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
#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/GlobalConfig.h"

#include "gtest/gtest.h"

using namespace cppmicroservices;

// type used to track move vs. copy constructor calls in the AnyMove test
struct MyType final {

    MyType() = default;
    ~MyType() = default;

    MyType(const MyType&) {
        ++copies;
    }
    MyType& operator =(const MyType&) {
        ++copies;
        return *this;
    }

    MyType(MyType&&) {
        ++moves;
    }
    MyType& operator =(MyType&&) {
        ++moves;
        return *this;
    }

    static unsigned long copies;
    static unsigned long moves;
};
std::ostream& operator<<(std::ostream& o, MyType const&) { return o; }

unsigned long MyType::copies = 0;
unsigned long MyType::moves = 0;

template <typename T>
void
TestUnsafeAnyCast(Any& anyObj, T val)
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
    EXPECT_LT(any_cast<float>(anyFloat) - 0.2f, std::numeric_limits<float>::epsilon());
    EXPECT_EQ(anyFloat.ToString(), "0.2");
    EXPECT_EQ(anyFloat.ToJSON(), "0.2");
    TestUnsafeAnyCast<float>(anyFloat, 0.2f);
}

TEST(AnyTest, AnyDouble)
{
    Any anyDouble = 0.5;
    EXPECT_EQ(anyDouble.Type(), typeid(double));
    EXPECT_LT(any_cast<double>(anyDouble) - 0.5, std::numeric_limits<double>::epsilon());
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
    std::map<std::string, int32_t> map = {
        {std::string("one"), 1},
        {std::string("two"), 2}
    };
    Any anyMap = map;
    EXPECT_EQ(anyMap.Type(), typeid(std::map<std::string, int32_t>));
    EXPECT_EQ((any_cast<std::map<std::string, int32_t>>(anyMap)), map);
    EXPECT_EQ(anyMap.ToString(), "{one : 1, two : 2}");
    EXPECT_EQ(anyMap.ToJSON(), "{\"one\" : 1, \"two\" : 2}");
}

TEST(AnyTest, AnyMapAny)
{
    std::map<int32_t, Any> map = {
        {1,                    0.3},
        {3, std::string("bonjour")}
    };
    Any anyMap = map;
    EXPECT_EQ(anyMap.Type(), typeid(std::map<int32_t, Any>));
    EXPECT_EQ(anyMap.ToString(), "{1 : 0.3, 3 : bonjour}");
    EXPECT_EQ(anyMap.ToJSON(), "{\"1\" : 0.3, \"3\" : \"bonjour\"}");
}

TEST(AnyTest, AnyStringEscapeCharacters)
{
    Any anyString = std::string("\"\\\b\f\n\r\t\x1f");
    EXPECT_EQ(anyString.ToJSON(), "\"\\\"\\\\\\b\\f\\n\\r\\t\\u001f\"");
}

TEST(AnyTest, AnyToJSONWithFormatting)
{
    std::map<int, Any> emptyMap;
    std::vector<int> emptyVector;

    std::map<int32_t, Any> mapToInsert = {
        {1,                    0.3},
        {3, std::string("bonjour")},
        {4,               emptyMap},
        {5,            emptyVector}
    };
    std::vector<int32_t> numbers { 9, 8, 7 };
    std::map<std::string, Any> map {
        {"number",           5},
        {"vector",     numbers},
        {   "map", mapToInsert}
    };

    Any anyMap = map;
    std::string expected = R"({
    "map" : {
        "1" : 0.3, 
        "3" : "bonjour", 
        "4" : {}, 
        "5" : []
    }, 
    "number" : 5, 
    "vector" : [
        9,
        8,
        7
    ]
})";
    EXPECT_EQ(anyMap.ToJSON(true), expected);
}

TEST(AnyTest, AnyMapComplex)
{
    std::map<int32_t, Any> mapToInsert = {
        {1,                    0.3},
        {3, std::string("bonjour")}
    };
    std::vector<int32_t> numbers = { 9, 8, 7 };
    std::map<std::string, Any> map = {
        {"number",           5},
        {"vector",     numbers},
        {   "map", mapToInsert}
    };

    std::string toStringRes = "{map : {1 : 0.3, 3 : bonjour}, number : 5, vector : [9,8,7]}";
    std::string toJSONRes = "{\"map\" : {\"1\" : 0.3, \"3\" : \"bonjour\"}, "
                            "\"number\" : 5, \"vector\" : [9,8,7]}";

    Any anyMap = map;
    EXPECT_EQ(anyMap.Type(), typeid(std::map<std::string, Any>));
    EXPECT_EQ(anyMap.ToString(), toStringRes);
    EXPECT_EQ(anyMap.ToJSON(), toJSONRes);
}

TEST(AnyTest, AnyBadAnyCastException)
{
    const Any uncastableConstAny(0.0);
    Any uncastableAny(0.0);

    EXPECT_THROW(any_cast<std::string>(uncastableConstAny), cppmicroservices::BadAnyCastException);
    EXPECT_THROW(any_cast<std::string>(uncastableAny), cppmicroservices::BadAnyCastException);
    EXPECT_THROW(ref_any_cast<std::string>(uncastableConstAny), cppmicroservices::BadAnyCastException);
    EXPECT_THROW(ref_any_cast<std::string>(uncastableAny), cppmicroservices::BadAnyCastException);
}

struct no_eq
{
    bool operator==(no_eq const&) const = delete;
    friend std::ostream&
    operator<<(std::ostream& os, no_eq const&)
    {
        os << "OOPS";
        return os;
    }
};
TEST(AnyTest, AnyEquality)
{
    EXPECT_EQ(Any(std::string("A")), Any(std::string("A")));
    EXPECT_NE(Any(std::string("A")), Any(std::string("B")));
    EXPECT_NE(Any(1), Any(std::string("A")));
    EXPECT_EQ(Any(1), Any(1));
    EXPECT_NE(Any(1), Any(2));
    EXPECT_EQ(Any(true), Any(true));
    EXPECT_NE(Any(true), Any(false));
    EXPECT_NE(Any(1), Any(true)); // type mismatch should never be equal
    EXPECT_NE(Any(0), Any(false));
    EXPECT_EQ(Any(1.5), Any(1.5));
    EXPECT_NE(Any(1.5), Any(1.6));

    Any no_eq_operator { no_eq() };
    EXPECT_NE(no_eq_operator,
              no_eq_operator); // Since no_eq has the equality operator deleted, Any,
                               // by design, always returns FALSE when equality is
                               // checked.

    AnyMap lhs(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    lhs["int"] = 1;
    lhs["float"] = 2.5;
    lhs["string"] = std::string("string");
    lhs["bool"] = true;
    AnyMap submap(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    submap["a"] = std::string("a");
    submap["b"] = std::string("b");
    lhs["submap"] = submap;

    AnyMap rhs = lhs;    // make a copy of lhs
    EXPECT_EQ(lhs, rhs); // they should be equal

    rhs["int"] = 2;
    EXPECT_NE(lhs, rhs); // they should not be equal after modifying the rhs.
    rhs["int"] = 1;
    EXPECT_EQ(lhs, rhs); // now they should be equal again
    rhs.erase("int");
    EXPECT_NE(lhs, rhs); // and finally, with the "int" element erased, they should not be equal 
                         // anymore.
}

TEST(AnyTest, AnyMove)
{
    cppmicroservices::AnyMap anyMap;
    anyMap.emplace("key1", MyType{}); // default

    cppmicroservices::AnyMap anyMap2;
    anyMap2.emplace("key2", MyType{}); // default

    anyMap.emplace("key3", std::move(anyMap2)); // move

    Any a1{ MyType{} }; // move
    Any a2 { a1 }; // copy
    Any a3 { std::move(a1) }; // move

    MyType m1;
    Any a4{ m1 }; // copy
    Any a5{ std::move(m1) }; // move
    
    EXPECT_EQ(2, MyType::copies);
    EXPECT_EQ(4, MyType::moves);
}
