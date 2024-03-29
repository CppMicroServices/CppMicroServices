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

#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/Any.h"
#include "cppmicroservices/Framework.h"
#include "gtest/gtest.h"

using namespace cppmicroservices;

TEST(FrameworkFactoryTest, testFrameworkInstantiation)
{
    auto f = FrameworkFactory().NewFramework();
    ASSERT_TRUE(f) << "Unique Test Framework must be instantiated";

    auto f1 = FrameworkFactory().NewFramework();
    ASSERT_NE(f, f1) << "Unique Test Framework must be instantiated";

    FrameworkConfiguration configuration;
    configuration["org.osgi.framework.security"] = std::string("osgi");
    configuration["org.osgi.framework.startlevel.beginning"] = 0;
    configuration["org.osgi.framework.bsnversion"] = std::string("single");
    configuration["org.osgi.framework.custom1"] = std::string("foo");
    configuration["org.osgi.framework.custom2"] = std::string("bar");

    auto f2 = FrameworkFactory().NewFramework(configuration);
    ASSERT_TRUE(f2) << "Test Framework must be instantiated with configuration";

    auto f3 = FrameworkFactory().NewFramework(std::unordered_map<std::string, cppmicroservices::Any>(), &std::clog);
    ASSERT_TRUE(f3) << "Test Framework must instantiated with default configuration "
                       "and custom logger";
}
