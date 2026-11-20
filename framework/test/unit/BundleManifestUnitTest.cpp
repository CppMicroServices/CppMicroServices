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

// White-box tests for the framework-private BundleManifest class. In
// usFrameworkTests, coverage like this needs a dedicated test bundle per
// malformed manifest (libMWithInvalidVersion, libMWithEmptyBundleName, ...).
// Here the parser is driven directly from in-memory JSON.

#include "BundleManifest.h"

#include "cppmicroservices/Any.h"
#include "cppmicroservices/AnyMap.h"

#include "gtest/gtest.h"

#include <sstream>
#include <stdexcept>

namespace cppmicroservices
{
    namespace
    {
        std::unique_ptr<BundleManifest>
        Parse(std::string const& json)
        {
            auto manifest = std::make_unique<BundleManifest>();
            std::istringstream is(json);
            manifest->Parse(is);
            return manifest;
        }
    } // namespace

    TEST(BundleManifestUnitTest, ParsesScalarHeaders)
    {
        auto m = Parse(R"({
            "bundle.symbolic_name": "test_bundle",
            "bundle.version": "1.2.3",
            "flag": true,
            "int": 5,
            "double": 1.5
        })");

        EXPECT_TRUE(m->Contains("bundle.symbolic_name"));
        EXPECT_EQ("test_bundle", any_cast<std::string>(m->GetValue("bundle.symbolic_name")));
        EXPECT_EQ("1.2.3", any_cast<std::string>(m->GetValue("bundle.version")));
        EXPECT_TRUE(any_cast<bool>(m->GetValue("flag")));
        EXPECT_EQ(5, any_cast<int>(m->GetValue("int")));
        EXPECT_DOUBLE_EQ(1.5, any_cast<double>(m->GetValue("double")));
        EXPECT_FALSE(m->Contains("missing"));
        EXPECT_TRUE(m->GetValue("missing").Empty());
    }

    TEST(BundleManifestUnitTest, TopLevelKeysAreCaseInsensitive)
    {
        auto m = Parse(R"({ "Bundle.Symbolic_Name": "test_bundle" })");

        EXPECT_TRUE(m->Contains("bundle.symbolic_name"));
        EXPECT_TRUE(m->Contains("BUNDLE.SYMBOLIC_NAME"));
    }

    TEST(BundleManifestUnitTest, ParsesNestedObjectsAndArrays)
    {
        auto m = Parse(R"({
            "bundle.symbolic_name": "test_bundle",
            "nested": { "list": [1, "two", false], "deeper": { "key": "value" } }
        })");

        auto nestedAny = m->GetValue("nested");
        auto const& nested = ref_any_cast<AnyMap>(nestedAny);
        auto const& list = ref_any_cast<std::vector<Any>>(nested.at("list"));
        ASSERT_EQ(3u, list.size());
        EXPECT_EQ(1, any_cast<int>(list[0]));
        EXPECT_EQ("two", any_cast<std::string>(list[1]));
        EXPECT_FALSE(any_cast<bool>(list[2]));
        EXPECT_EQ("value", any_cast<std::string>(nested.AtCompoundKey("deeper.key")));
    }

    TEST(BundleManifestUnitTest, InvalidJsonThrows)
    {
        EXPECT_THROW(Parse(R"({ "bundle.symbolic_name": )"), std::runtime_error);
        EXPECT_THROW(Parse(""), std::runtime_error);
    }

    TEST(BundleManifestUnitTest, NonObjectRootThrows)
    {
        EXPECT_THROW(Parse(R"([ "not", "an", "object" ])"), std::runtime_error);
        EXPECT_THROW(Parse(R"("just a string")"), std::runtime_error);
    }

    TEST(BundleManifestUnitTest, ConstructFromAnyMap)
    {
        AnyMap headers(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
        headers["bundle.symbolic_name"] = std::string("injected");
        BundleManifest m(headers);

        EXPECT_EQ("injected", any_cast<std::string>(m.GetValue("bundle.symbolic_name")));
        EXPECT_EQ(1u, m.GetHeaders().size());
    }
} // namespace cppmicroservices
