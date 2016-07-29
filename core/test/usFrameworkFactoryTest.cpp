/*=============================================================================

Library: CppMicroServices

Copyright (c) The CppMicroServices developers. See the COPYRIGHT
file at the top-level directory of this distribution and at
https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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

#include <usFrameworkFactory.h>
#include <usFramework.h>
#include <usAny.h>

#include "usTestingMacros.h"
#include "usTestingConfig.h"

#include <iostream>
#include <string>
#include <map>

using namespace us;

int usFrameworkFactoryTest(int /*argc*/, char* /*argv*/[])
{
    US_TEST_BEGIN("FrameworkFactoryTest");

    auto f = FrameworkFactory().NewFramework();

    US_TEST_CONDITION(f, "Test Framework instantiation");

    auto f1 = FrameworkFactory().NewFramework();

    US_TEST_CONDITION(f != f1, "Test unique Framework instantiation");

    std::map < std::string, Any > configuration;
    configuration["org.osgi.framework.security"] = std::string("osgi");
    configuration["org.osgi.framework.startlevel.beginning"] = 0;
    configuration["org.osgi.framework.bsnversion"] = std::string("single");
    configuration["org.osgi.framework.custom1"] = std::string("foo");
    configuration["org.osgi.framework.custom2"] = std::string("bar");

    auto f2 = FrameworkFactory().NewFramework(configuration);

    US_TEST_CONDITION(f2, "Test Framework instantiation with configuration");

	auto f3 = FrameworkFactory().NewFramework(std::map<std::string, us::Any>(), &std::clog);

	US_TEST_CONDITION(f3, "Test Framework instantiation with default configuration and custom logger");

    US_TEST_END()
}
