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

#include "usTestingMacros.h"
#include "usTestingConfig.h"

#include <string>
#include <map>

using namespace us;

int usFrameworkFactoryTest(int /*argc*/, char* /*argv*/[])
{
    US_TEST_BEGIN("FrameworkFactoryTest");

    FrameworkFactory factory;

    auto f = factory.NewFramework();

    US_TEST_CONDITION(f, "Test Framework instantiation");

    auto f1 = factory.NewFramework();

    US_TEST_CONDITION(f != f1, "Test unique Framework instantiation");

    std::map < std::string, std::string > configuration;
    configuration.insert(std::pair<std::string, std::string>("org.cppmicroservices.framework.security","osgi"));
    configuration.insert(std::pair<std::string, std::string>("org.cppmicroservices.framework.startlevel.beginning", "0"));
    configuration.insert(std::pair<std::string, std::string>("org.cppmicroservices.framework.bsnversion", "single"));
    configuration.insert(std::pair<std::string, std::string>("org.cppmicroservices.framework.custom1", "foo"));
    configuration.insert(std::pair<std::string, std::string>("org.cppmicroservices.framework.custom2", "bar"));

    auto f2 = factory.NewFramework(configuration);

    US_TEST_CONDITION(f2, "Test Framework instantiation with configuration");

    delete f;
    delete f1;
    delete f2;

    US_TEST_END()
}
