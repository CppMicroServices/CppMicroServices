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

#include <string>

#include "cppmicroservices/em/EMConstants.hpp"
#include "cppmicroservices/em/Event.hpp"

#include "cppmicroservices/Any.h"
#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/LDAPFilter.h"

#include "gtest/gtest.h"

using cppmicroservices::service::em::Event;
using cppmicroservices::service::em::EventProperties;
namespace emc = cppmicroservices::em::Constants;

namespace
{
    TEST(EventTest, TestConstructionValidTopic)
    {
        EXPECT_NO_THROW({ Event("some/valid/topic"); });
    }

    TEST(EventTest, TestInvalidConstructionInvalidTopic)
    { 
        EXPECT_THROW({ Event("/not/a/valid/topic"); }, std::logic_error);
    }

    TEST(EventTest, TestConstructionValidTopicWithProps)
    {
        EXPECT_NO_THROW({
            EventProperties props({
                {"tProp1", std::string("hi")}
            });

            Event("some/valid/topic", props);
        });
    }

    TEST(EventTest, TestConstructionInvalidTopicWithProps)
    {
        EXPECT_THROW(
            {
                EventProperties props({
                    {"tProp1", std::string("hi")}
                });

                Event("/not/a/valid/topic", props);
            },
            std::logic_error);
    }

    TEST(EventTest, TestCopyConstruction)
    {
        EXPECT_NO_THROW({
            EventProperties props({
                {"tProp1", std::string("hi")}
            });
            Event evt1("some/valid/topic", props);
            Event evt2(evt1);

            EXPECT_EQ(evt1, evt2);
        });
    }

    TEST(EventTest, TestEquality)
    {
        // Same topic, no props
        Event evt_0a("evt_0/1");
        Event evt_0b("evt_0/1");

        ASSERT_EQ(evt_0a, evt_0b);

        // Different topic, no props
        Event evt_1a("evt_1/a");
        Event evt_1b("evt_1/b");

        ASSERT_NE(evt_1a, evt_1b);

        // Same topic, same props
        EventProperties props_2({
            {"tProp1", std::string("bonjour")}
        });
        Event evt_2a("evt_2", props_2);
        Event evt_2b("evt_2", props_2);

        ASSERT_EQ(evt_2a, evt_2b);

        // Same topic, diff props size
        EventProperties props_3a(props_2);
        EventProperties props_3b({
            {"tProp1", std::string("bonjour")},
            {"tProp2",   std::string("hello")}
        });
        Event evt_3a("evt_3", props_3a);
        Event evt_3b("evt_3", props_3b);

        ASSERT_NE(evt_3a, evt_3b);

        // Same topic, same props size but different keys
        EventProperties props_4a({
            {"tProp1", std::string("bonjour")}
        });
        EventProperties props_4b({
            {"tPropDiff", std::string("bonjour")}
        });
        Event evt_4a("evt_4", props_4a);
        Event evt_4b("evt_4", props_4b);

        ASSERT_NE(evt_4a, evt_4b);

        // Same topic, same props size, same prop keys but different values
        EventProperties props_5a({
            {"tProp1", std::string("bonjour")}
        });
        EventProperties props_5b({
            {"tProp1", std::string("hello")}
        });
        Event evt_5a("evt_5", props_5a);
        Event evt_5b("evt_5", props_5b);

        ASSERT_NE(evt_5a, evt_5b);
    }

    TEST(EventTest, TestProperties)
    {
        EventProperties props({
            { emc::SERVICE_ID,                                 1},
            {emc::OBJECTCLASS,       std::string("testobjclass")},
            {  emc::EXCEPTION, std::string("std::runtime_error")}
        });

        Event evt("my/event/topic", props);

        // ContainsProperty()
        ASSERT_TRUE(evt.ContainsProperty(emc::SERVICE_ID));
        ASSERT_FALSE(evt.ContainsProperty("not.a.real.prop"));

        // GetProperty()
        ASSERT_FALSE(evt.GetProperty(emc::OBJECTCLASS).Empty());
        ASSERT_TRUE(evt.GetProperty("not.a.real.prop").Empty());

        // GetProperties()
        ASSERT_EQ(evt.GetProperties(), cppmicroservices::AnyMap(props));
        EventProperties diffProps({
            {"tProp1", std::string("hi")}
        });
        ASSERT_NE(evt.GetProperties(), diffProps);

        // GetPropertyNames()
        auto propNames = evt.GetPropertyNames();
        ASSERT_EQ(propNames.size(), 3ul);
        ASSERT_NE(std::find(propNames.begin(), propNames.end(), emc::SERVICE_ID), propNames.end());
        ASSERT_NE(std::find(propNames.begin(), propNames.end(), emc::OBJECTCLASS), propNames.end());
        ASSERT_NE(std::find(propNames.begin(), propNames.end(), emc::EXCEPTION), propNames.end());
    }

    TEST(EventTest, TestGetTopic)
    {
        Event evt("my/event/topic");
        ASSERT_EQ(evt.GetTopic(), "my/event/topic");
    }

    TEST(EventTest, TestFilterMatches)
    {
        EventProperties props_1({
            {emc::BUNDLE_SYMBOLICNAME, std::string("my_cool_bundle")},
        });
        Event evt1("my/event/topic", props_1);

        cppmicroservices::LDAPFilter filter("(bundle.symbolicName=my_cool_bundle)");
        ASSERT_TRUE(evt1.Matches(filter));

        EventProperties props_2({
            {emc::BUNDLE_SYMBOLICNAME, std::string("my_not_cool_bundle")}
        });
        Event evt2("my/event/topic", props_2);

        ASSERT_FALSE(evt2.Matches(filter));
    }

    TEST(EventTest, TestInvalidTopicCharacters)
    {
        EXPECT_THROW({ Event("/not/a/valid/topic+"); }, std::logic_error);
        EXPECT_THROW({ Event("/not/a/vali;d/topic"); }, std::logic_error);
        EXPECT_THROW({ Event("/not/a/valid\topic"); }, std::logic_error);
        EXPECT_THROW({ Event("/not/a!/valid/topic"); }, std::logic_error);
    }

} // namespace
