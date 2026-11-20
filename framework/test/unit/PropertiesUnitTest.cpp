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

// White-box tests for the framework-private Properties class (the storage
// behind service properties).

#include "Properties.h"

#include "cppmicroservices/Any.h"
#include "cppmicroservices/AnyMap.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <stdexcept>

namespace cppmicroservices
{
    namespace
    {
        AnyMap
        MakeMap(AnyMap::map_type type)
        {
            AnyMap m(type);
            m["Service.Ranking"] = 5;
            m["objectclass"] = std::string("IFoo");
            return m;
        }
    } // namespace

    class PropertiesMapTypeTest : public ::testing::TestWithParam<AnyMap::map_type>
    {
    };

    TEST_P(PropertiesMapTypeTest, ExactCaseLookupSucceeds)
    {
        Properties props(MakeMap(GetParam()));

        auto [value, found] = props.Value_unlocked("Service.Ranking", true);
        EXPECT_TRUE(found);
        EXPECT_EQ(5, any_cast<int>(value));
        EXPECT_EQ(5, any_cast<int>(props.ValueByRef_unlocked("Service.Ranking", true)));
    }

    TEST_P(PropertiesMapTypeTest, CaseInsensitiveLookupByDefault)
    {
        Properties props(MakeMap(GetParam()));

        auto [value, found] = props.Value_unlocked("SERVICE.RANKING");
        EXPECT_TRUE(found);
        EXPECT_EQ(5, any_cast<int>(value));
        EXPECT_EQ("IFoo", any_cast<std::string>(props.ValueByRef_unlocked("ObjectClass")));
    }

    TEST_P(PropertiesMapTypeTest, MatchCaseRejectsDifferentCase)
    {
        Properties props(MakeMap(GetParam()));

        EXPECT_FALSE(props.Value_unlocked("SERVICE.RANKING", true).second);
        EXPECT_TRUE(props.ValueByRef_unlocked("SERVICE.RANKING", true).Empty());
    }

    TEST_P(PropertiesMapTypeTest, MissingKeyReturnsEmpty)
    {
        Properties props(MakeMap(GetParam()));

        EXPECT_FALSE(props.Value_unlocked("nope").second);
        EXPECT_TRUE(props.ValueByRef_unlocked("nope").Empty());
    }

    TEST_P(PropertiesMapTypeTest, KeysAndClear)
    {
        Properties props(MakeMap(GetParam()));

        auto keys = props.Keys_unlocked();
        ASSERT_EQ(2u, keys.size());
        EXPECT_NE(keys.end(), std::find(keys.begin(), keys.end(), "Service.Ranking"));
        EXPECT_NE(keys.end(), std::find(keys.begin(), keys.end(), "objectclass"));

        props.Clear_unlocked();
        EXPECT_TRUE(props.Keys_unlocked().empty());
    }

    INSTANTIATE_TEST_SUITE_P(AllMapTypes,
                             PropertiesMapTypeTest,
                             ::testing::Values(AnyMap::ORDERED_MAP,
                                               AnyMap::UNORDERED_MAP,
                                               AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS));

    TEST(PropertiesUnitTest, KeysDifferingOnlyInCaseAreRejected)
    {
        AnyMap ordered(AnyMap::ORDERED_MAP);
        ordered["key"] = 1;
        ordered["KEY"] = 2;
        EXPECT_THROW(Properties { ordered }, std::runtime_error);

        AnyMap unordered(AnyMap::UNORDERED_MAP);
        unordered["key"] = 1;
        unordered["KEY"] = 2;
        EXPECT_THROW(Properties { std::move(unordered) }, std::runtime_error);
    }
} // namespace cppmicroservices
