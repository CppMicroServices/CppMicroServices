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

#include "cppmicroservices/AnyMap.h"

#include "gtest/gtest.h"
#include "Properties.h"
#include "TestUtils.h"
#include "TestingConfig.h"

namespace cppmicroservices
{
    class PropertiesTest : public ::testing::Test
    {
    };

    /*
     * Properties constructor should throw if case-insensitive AnyMap has
     * duplicate key names.
     */
    TEST_F(PropertiesTest, ValidationFailure)
    {
        AnyMap map = AnyMap(AnyMap::UNORDERED_MAP, {
            { "hello", std::string("world") },
            { "HELLO", std::string("WORLD") }
        });
        EXPECT_THROW({ Properties props(map); }, std::runtime_error);
    }

    /*
     * Test Value behavior with all four branch paths based on input
     * AnyMap type.
     */
    TEST_F(PropertiesTest, Value)
    {
        auto test = [](AnyMap& map)
        {
            Properties props(map);
            {
                auto result = props.Value_unlocked("missing", false);
                ASSERT_FALSE(result.second);
            }

            {
                auto result = props.Value_unlocked("HELLO", true);
                ASSERT_FALSE(result.second);
            }

            {
                auto result = props.Value_unlocked("hello", false);
                std::string tmp = any_cast<std::string>(result.first);
                std::string val = "world";
                ASSERT_STREQ(tmp.c_str(), val.c_str());
            }

            {
                auto result = props.Value_unlocked("HELLO", false);
                std::string tmp = any_cast<std::string>(result.first);
                std::string val = "world";
                ASSERT_STREQ(tmp.c_str(), val.c_str());
            }

            ASSERT_NO_THROW(props.Clear_unlocked());
        };

        {
            AnyMap map {
                AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS, {
                    { "hello", std::string("world") }
                }
            };
            ASSERT_NO_THROW({ test(map); });
        }

        {
            AnyMap map {
                AnyMap::UNORDERED_MAP, {
                    { "hello", std::string("world") }
                }
            };
            ASSERT_NO_THROW({ test(map); });
        }

        {
            AnyMap map {
                AnyMap::ORDERED_MAP, {
                    { "hello", std::string("world") }
                }
            };
            ASSERT_NO_THROW({ test(map); });
        }
    }

    /*
     * Test ValueByRef behavior with all four branch paths based on input
     * AnyMap type.
     */
    TEST_F(PropertiesTest, ValueByRef)
    {
        auto test = [](AnyMap& map)
        {
            Properties props(map);
            {
                Any emptyAny;
                Any const& ref = props.ValueByRef_unlocked("missing", false);
                ASSERT_EQ(ref._content.get(), nullptr);
            }

            {
                Any emptyAny;
                Any const& ref = props.ValueByRef_unlocked("HELLO", true);
                ASSERT_EQ(ref._content.get(), nullptr);
            }

            {
                Any const& ref = props.ValueByRef_unlocked("hello", false);
                std::string tmp = any_cast<std::string>(ref);
                std::string val = "world";
                ASSERT_STREQ(tmp.c_str(), val.c_str());
            }

            {
                Any const& ref = props.ValueByRef_unlocked("HELLO", false);
                std::string tmp = any_cast<std::string>(ref);
                std::string val = "world";
                ASSERT_STREQ(tmp.c_str(), val.c_str());
            }

            ASSERT_NO_THROW(props.Clear_unlocked());
        };

        {
            AnyMap map {
                AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS, {
                    { "hello", std::string("world") }
                }
            };
            ASSERT_NO_THROW({ test(map); });
        }

        {
            AnyMap map {
                AnyMap::UNORDERED_MAP, {
                    { "hello", std::string("world") }
                }
            };
            ASSERT_NO_THROW({ test(map); });
        }

        {
            AnyMap map {
                AnyMap::ORDERED_MAP, {
                    { "hello", std::string("world") }
                }
            };
            ASSERT_NO_THROW({ test(map); });
        }
    }
} // namespace cppmicroservices
