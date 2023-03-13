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

#include "gtest/gtest.h"

#include "cppmicroservices/cm/ConfigurationException.hpp"

using cppmicroservices::service::cm::ConfigurationException;

namespace
{

    constexpr auto REASON = "A reason";
    constexpr auto PROPERTY = "Foo";

    /**
     * This test point is used to verify that the ConfigurationException
     * behaves as expected.
     */
    TEST(ConfigurationException, VerifyConstructorAndGetters)
    {
        ConfigurationException e { REASON, PROPERTY };
        ASSERT_EQ(REASON, e.GetReason());
        ASSERT_EQ(PROPERTY, e.GetProperty());

        ConfigurationException e2 { REASON };
        ASSERT_EQ(REASON, e2.GetReason());
        ASSERT_EQ("", e2.GetProperty());
    }

    /**
     * This test point is used to verify that the ConfigurationException
     * can be thrown and caught as expected.
     */
    TEST(ConfigurationException, VerifyThrowAndCatch)
    {
        ASSERT_THROW(
            {
                try
                {
                    throw ConfigurationException(REASON, PROPERTY);
                }
                catch (const ConfigurationException& e)
                {
                    ASSERT_EQ(REASON, e.GetReason());
                    ASSERT_EQ(PROPERTY, e.GetProperty());
                    throw;
                }
            },
            ConfigurationException);
        ASSERT_THROW(
            {
                try
                {
                    throw ConfigurationException(REASON, PROPERTY);
                }
                catch (const std::runtime_error& e)
                {
                    auto what = std::string { e.what() };
                    ASSERT_NE("", what);
                    ASSERT_NE(std::string::npos, what.find(REASON));
                    ASSERT_NE(std::string::npos, what.find(PROPERTY));
                    throw;
                }
            },
            std::runtime_error);
        ASSERT_THROW(
            {
                try
                {
                    throw ConfigurationException(REASON);
                }
                catch (const std::exception& e)
                {
                    auto what = std::string { e.what() };
                    ASSERT_NE("", what);
                    ASSERT_NE(std::string::npos, what.find(REASON));
                    ASSERT_EQ(std::string::npos, what.find("property"));
                    throw;
                }
            },
            std::exception);
    }
} // namespace
