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

#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/Bundle.h"

#include "TestUtils.h"

#include "gtest/gtest.h"

TEST(SecurityExceptionTest, ctor)
{

    cppmicroservices::Bundle b;
    std::string excMessage { "foo" };
    cppmicroservices::SecurityException secException(excMessage, b);

    cppmicroservices::SecurityException copyOfException = secException;

    ASSERT_EQ(b, secException.GetBundle());
    ASSERT_STREQ(excMessage.c_str(), secException.what());

    ASSERT_EQ(b, copyOfException.GetBundle());
    ASSERT_STREQ(excMessage.c_str(), copyOfException.what());
}
