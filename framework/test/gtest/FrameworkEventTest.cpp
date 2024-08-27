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

#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "TestUtils.h"
#include "TestingConfig.h"
#include "Mocks.h"
#include "MockUtils.h"
#include "gtest/gtest.h"

#include <iostream>
#include <typeinfo>

using namespace cppmicroservices;
using ::testing::_;
using ::testing::Eq;
using ::testing::Return;
using ::testing::AtLeast;

namespace
{

    std::string
    GetMessageFromStdExceptionPtr(const std::exception_ptr ptr)
    {
        if (ptr)
        {
            try
            {
                std::rethrow_exception(ptr);
            }
            catch (std::exception const& e)
            {
                return e.what();
            }
        }
        return std::string();
    }

} // end anonymous namespace

TEST(FrameworkEventTest, testFrameworkEvents)
{
    // The OSGi spec assigns specific values to event types for future extensibility.
    // Ensure we don't deviate from those assigned values.
    // Test assigned event type values
    ASSERT_EQ(FrameworkEvent::Type::FRAMEWORK_STARTED, static_cast<FrameworkEvent::Type>(1));
    ASSERT_EQ(FrameworkEvent::Type::FRAMEWORK_ERROR, static_cast<FrameworkEvent::Type>(2));
    ASSERT_EQ(FrameworkEvent::Type::FRAMEWORK_INFO, static_cast<FrameworkEvent::Type>(32));
    ASSERT_EQ(FrameworkEvent::Type::FRAMEWORK_WARNING, static_cast<FrameworkEvent::Type>(16));
    ASSERT_EQ(FrameworkEvent::Type::FRAMEWORK_STOPPED, static_cast<FrameworkEvent::Type>(64));
    ASSERT_EQ(FrameworkEvent::Type::FRAMEWORK_STOPPED_UPDATE, static_cast<FrameworkEvent::Type>(128));
    ASSERT_EQ(FrameworkEvent::Type::FRAMEWORK_WAIT_TIMEDOUT, static_cast<FrameworkEvent::Type>(512));

    // @todo mock the framework. We only need a Bundle object to construct a FrameworkEvent object.
    auto const f = FrameworkFactory().NewFramework();

    std::string default_exception_message(GetMessageFromStdExceptionPtr(nullptr));

    FrameworkEvent invalid_event;

    // Test for invalid FrameworkEvent construction.
    ASSERT_FALSE(invalid_event);
    // Test invalid event GetType()
    ASSERT_EQ(invalid_event.GetType(), FrameworkEvent::Type::FRAMEWORK_ERROR);
    // Test invalid event GetBundle()
    ASSERT_FALSE(invalid_event.GetBundle());
    // Test invalid event GetThrowable()
    ASSERT_EQ(GetMessageFromStdExceptionPtr(invalid_event.GetThrowable()), default_exception_message);

    FrameworkEvent error_event(FrameworkEvent::Type::FRAMEWORK_ERROR,
                               f,
                               "test framework error event",
                               std::make_exception_ptr(std::runtime_error("test exception")));

    // Test FrameworkEvent construction - error type
    ASSERT_TRUE(error_event);
    ASSERT_EQ(error_event.GetType(), FrameworkEvent::Type::FRAMEWORK_ERROR);
    ASSERT_EQ(error_event.GetBundle(), f);
    ASSERT_EQ(GetMessageFromStdExceptionPtr(error_event.GetThrowable()), std::string("test exception"));

    bool exception_caught = false;
    try
    {
        std::rethrow_exception(error_event.GetThrowable());
    }
    catch (std::exception const& ex)
    {
        exception_caught = true;

        // Test FrameworkEvent::Type::FRAMEWORK_ERROR exception
        ASSERT_EQ(ex.what(), std::string("test exception"));
        // Test that the correct exception type was thrown
        ASSERT_EQ(std::string(typeid(std::runtime_error).name()), typeid(ex).name());
    }

    // Test throw/catch a FrameworkEvent exception
    ASSERT_TRUE(exception_caught);

    FrameworkEvent info_event(FrameworkEvent::Type::FRAMEWORK_INFO, f, "test info framework event");
    // Test FrameworkEvent construction - info type
    ASSERT_TRUE(info_event);
    ASSERT_EQ(info_event.GetType(), FrameworkEvent::Type::FRAMEWORK_INFO);
    ASSERT_EQ(info_event.GetBundle(), f);
    ASSERT_EQ(GetMessageFromStdExceptionPtr(info_event.GetThrowable()), default_exception_message);

    FrameworkEvent warn_event(FrameworkEvent::Type::FRAMEWORK_WARNING, f, "test warning framework event");
    // Test FrameworkEvent construction - warning type
    ASSERT_TRUE(warn_event);
    ASSERT_EQ(warn_event.GetType(), FrameworkEvent::Type::FRAMEWORK_WARNING);
    ASSERT_EQ(warn_event.GetBundle(), f);
    ASSERT_EQ(GetMessageFromStdExceptionPtr(warn_event.GetThrowable()), default_exception_message);

    FrameworkEvent unknown_event(static_cast<FrameworkEvent::Type>(127), f, "test unknown framework event");
    // Test FrameworkEvent construction - unknown type
    ASSERT_TRUE(unknown_event);
    ASSERT_EQ(unknown_event.GetType(), static_cast<FrameworkEvent::Type>(127));
    ASSERT_EQ(unknown_event.GetBundle(), f);
    ASSERT_EQ(GetMessageFromStdExceptionPtr(unknown_event.GetThrowable()), default_exception_message);

    // copy test
    FrameworkEvent dup_error_event(error_event);
    ASSERT_EQ(dup_error_event, error_event);

    // copy assignment test
    dup_error_event = invalid_event;
    ASSERT_FALSE(dup_error_event == error_event);

    dup_error_event = FrameworkEvent(FrameworkEvent::Type::FRAMEWORK_STARTED, f, "");
    ASSERT_FALSE(dup_error_event == error_event);
}

TEST(FrameworkEventTest, testFrameworkEventStreamOperator)
{
    FrameworkEvent frameworkEvent;
    ASSERT_EQ(0, frameworkEvent.GetMessage().length());

    std::vector<FrameworkEvent::Type> frameworkTypes = {
        FrameworkEvent::Type::FRAMEWORK_STARTED,
        FrameworkEvent::Type::FRAMEWORK_ERROR,
        FrameworkEvent::Type::FRAMEWORK_WARNING,
        FrameworkEvent::Type::FRAMEWORK_INFO,
        FrameworkEvent::Type::FRAMEWORK_STOPPED,
        FrameworkEvent::Type::FRAMEWORK_STOPPED_UPDATE,
        FrameworkEvent::Type::FRAMEWORK_WAIT_TIMEDOUT
    };

    std::string bundleName = "MockBundle";
    MockedEnvironment mockEnv(false);
    MockBundleStorageMemory* bundleStorage = mockEnv.bundleStorage;
    AnyMap manifest = AnyMap({
        { "bundle.activator", Any(true) },
        { "bundle.symbolic_name", Any(std::string(bundleName)) },
    });

    std::vector<std::string> files = {bundleName};
    auto resCont = std::make_shared<MockBundleResourceContainer>();
    ON_CALL(*resCont, GetTopLevelDirs())
        .WillByDefault(Return(files));
    EXPECT_CALL(*resCont, GetTopLevelDirs()).Times(1);

    ON_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(bundleName), _))
        .WillByDefault(Return(std::make_shared<MockBundleArchive>(
            bundleStorage,
            resCont,
            bundleName, bundleName, 1,
            manifest
        )));
    EXPECT_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(bundleName), _)).Times(1);

    auto bundle = mockEnv.Install(bundleName, manifest, resCont).at(0);

    std::ostringstream buf;
    std::string message = "hello world";
    std::exception_ptr e = std::make_exception_ptr(std::runtime_error("placeholder"));
    for (auto type : frameworkTypes)
    {
        FrameworkEvent evt(type, bundle, message, e);
        buf << evt << "\n";
    }
    std::string goal = "STARTED\n hello world\n Bundle[id=1, loc=MockBundle, name=MockBundle, state=INSTALLED]\n Exception: placeholder\nERROR\n hello world\n Bundle[id=1, loc=MockBundle, name=MockBundle, state=INSTALLED]\n Exception: placeholder\nWARNING\n hello world\n Bundle[id=1, loc=MockBundle, name=MockBundle, state=INSTALLED]\n Exception: placeholder\nINFO\n hello world\n Bundle[id=1, loc=MockBundle, name=MockBundle, state=INSTALLED]\n Exception: placeholder\nSTOPPED\n hello world\n Bundle[id=1, loc=MockBundle, name=MockBundle, state=INSTALLED]\n Exception: placeholder\nSTOPPED_UPDATE\n hello world\n Bundle[id=1, loc=MockBundle, name=MockBundle, state=INSTALLED]\n Exception: placeholder\nWAIT_TIMEDOUT\n hello world\n Bundle[id=1, loc=MockBundle, name=MockBundle, state=INSTALLED]\n Exception: placeholder\n";
    ASSERT_EQ(goal, buf.str());
}
