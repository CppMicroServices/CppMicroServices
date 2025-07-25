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

#include <sstream>

#include "gmock/gmock.h"

#include <cppmicroservices/Constants.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>

#include "../src/ConfigurationImpl.hpp"
#include "Mocks.hpp"

namespace cppmicroservices
{
    namespace cmimpl
    {

        MATCHER_P(AnyMapEquals, value, "")
        {
            std::ostringstream argString;
            std::ostringstream valueString;
            cppmicroservices::any_value_to_string(argString, arg);
            cppmicroservices::any_value_to_string(valueString, value);
            return (argString.str() == valueString.str());
        }

        TEST(TestConfigurationImpl, VerifyGetters)
        {
            auto mockConfigAdmin = std::make_shared<testing::NiceMock<MockConfigurationAdminPrivate>>();
            AnyMap props { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS };
            props["foo"] = std::string("bar");
            std::string pid { "test~instance" };
            std::string factoryPid { "test" };
            ConfigurationImpl conf { mockConfigAdmin.get(), pid, factoryPid, props };
            EXPECT_EQ(conf.GetPid(), pid);
            EXPECT_EQ(conf.GetFactoryPid(), factoryPid);
            EXPECT_THAT(conf.GetProperties(), AnyMapEquals(props));
        }

        TEST(TestConfigurationImpl, ThrowsWhenRemoved)
        {
            auto mockConfigAdmin = std::make_shared<testing::NiceMock<MockConfigurationAdminPrivate>>();
            AnyMap props { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS };
            props["foo"] = std::string("bar");
            std::string pid { "test~instance" };
            std::string factoryPid { "test" };
            ConfigurationImpl conf { mockConfigAdmin.get(), pid, factoryPid, props };
            auto validFuture = std::make_shared<ThreadpoolSafeFuturePrivate>();

            EXPECT_CALL(*mockConfigAdmin,
                        NotifyConfigurationRemoved(pid, reinterpret_cast<std::uintptr_t>(&conf), testing::_))
                .Times(1)
                .WillOnce(testing::Return(validFuture));
            ;
            EXPECT_NO_THROW(conf.Remove());
            EXPECT_THROW(conf.GetPid(), std::runtime_error);
            EXPECT_THROW(conf.GetFactoryPid(), std::runtime_error);
            EXPECT_THROW(conf.GetProperties(), std::runtime_error);
            EXPECT_THROW(conf.Update(props), std::runtime_error);
            EXPECT_THROW(conf.UpdateIfDifferent(props), std::runtime_error);
            EXPECT_THROW(conf.Remove(), std::runtime_error);
            EXPECT_THROW(conf.UpdateWithoutNotificationIfDifferent(props), std::runtime_error);
            EXPECT_THROW(conf.RemoveWithoutNotificationIfChangeCountEquals(0u), std::runtime_error);
            EXPECT_NO_THROW(conf.Invalidate());
        }

        TEST(TestConfigurationImpl, NoCallbacksAfterInvalidate)
        {
            auto mockConfigAdmin = std::make_shared<testing::NiceMock<MockConfigurationAdminPrivate>>();
            AnyMap props { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS };
            props["foo"] = std::string("bar");
            std::string pid { "test~instance" };
            std::string factoryPid { "test" };
            ConfigurationImpl conf { mockConfigAdmin.get(), pid, factoryPid, props };
            EXPECT_CALL(*mockConfigAdmin, NotifyConfigurationRemoved(testing::_, testing::_, testing::_)).Times(0);
            EXPECT_CALL(*mockConfigAdmin, NotifyConfigurationUpdated(testing::_, testing::_)).Times(0);
            EXPECT_NO_THROW(conf.Invalidate());
            EXPECT_NO_THROW(conf.Update(props));
            props["bar"] = std::string("foo");
            auto result = conf.UpdateIfDifferent(props);
            EXPECT_TRUE(result.first);
            EXPECT_NO_THROW(conf.Remove());
        }

        TEST(TestConfigurationImpl, VerifyUpdate)
        {
            auto mockConfigAdmin = std::make_shared<testing::NiceMock<MockConfigurationAdminPrivate>>();
            AnyMap props { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS };
            props["foo"] = std::string("bar");
            std::string pid { "test~instance" };
            std::string factoryPid { "test" };
            ConfigurationImpl conf { mockConfigAdmin.get(), pid, factoryPid, props };
            auto validFuture = std::make_shared<ThreadpoolSafeFuturePrivate>();

            EXPECT_CALL(*mockConfigAdmin, NotifyConfigurationUpdated(pid, testing::_))
                .Times(1)
                .WillOnce(testing::Return(validFuture));
            ;
            EXPECT_NO_THROW(conf.Update(props));
        }

        TEST(TestConfigurationImpl, VerifyUpdateIfDifferent)
        {
            auto mockConfigAdmin = std::make_shared<testing::NiceMock<MockConfigurationAdminPrivate>>();
            AnyMap props { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS };
            props["foo"] = std::string("bar");
            std::string pid { "test~instance" };
            std::string factoryPid { "test" };
            ConfigurationImpl conf { mockConfigAdmin.get(), pid, factoryPid, props };
            auto validFuture = std::make_shared<ThreadpoolSafeFuturePrivate>();

            EXPECT_CALL(*mockConfigAdmin, NotifyConfigurationUpdated(pid, testing::_))
                .Times(1)
                .WillOnce(testing::Return(validFuture));
            auto result = conf.UpdateIfDifferent(props);
            EXPECT_FALSE(result.first);
            props["bar"] = std::string("baz");
            result = conf.UpdateIfDifferent(props);
            EXPECT_TRUE(result.first);
        }

        TEST(TestConfigurationImpl, VerifyUpdateWithoutNotificationIfDifferent)
        {
            auto mockConfigAdmin = std::make_shared<testing::NiceMock<MockConfigurationAdminPrivate>>();
            AnyMap props { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS };
            props["foo"] = std::string("bar");
            std::string pid { "test~instance" };
            std::string factoryPid { "test" };
            ConfigurationImpl conf { mockConfigAdmin.get(), pid, factoryPid, props };
            EXPECT_CALL(*mockConfigAdmin, NotifyConfigurationUpdated(pid, testing::_)).Times(0);
            EXPECT_EQ(conf.UpdateWithoutNotificationIfDifferent(props), std::make_pair(false, 0ul));
            props["bar"] = std::string("baz");
            EXPECT_EQ(conf.UpdateWithoutNotificationIfDifferent(props), std::make_pair(true, 2ul));
        }

        TEST(TestConfigurationImpl, VerifyRemoveWithoutNotificationIfChangeCountEquals)
        {
            auto mockConfigAdmin = std::make_shared<testing::NiceMock<MockConfigurationAdminPrivate>>();
            AnyMap props { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS };
            props["foo"] = std::string("bar");
            std::string pid { "test~instance" };
            std::string factoryPid { "test" };
            ConfigurationImpl conf { mockConfigAdmin.get(), pid, factoryPid, props };
            EXPECT_CALL(*mockConfigAdmin, NotifyConfigurationRemoved(testing::_, testing::_, testing::_)).Times(0);
            EXPECT_FALSE(conf.RemoveWithoutNotificationIfChangeCountEquals(0ul));
            EXPECT_TRUE(conf.RemoveWithoutNotificationIfChangeCountEquals(1ul));
        }
    } // namespace cmimpl
} // namespace cppmicroservices
