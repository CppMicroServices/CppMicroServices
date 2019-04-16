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

#include "cppmicroservices/Bundle.h"
#include "gtest/gtest.h"
#include <stdexcept>

using namespace cppmicroservices;

TEST(InvalidBundle, GettingBundleIdFromInvalidBundle)
{
    Bundle b;
    EXPECT_THROW(b.GetBundleId(),std::invalid_argument);
}

TEST(InvalidBundle, GettingBundleLocationFromInvalidBundle)
{
    Bundle b;
    EXPECT_THROW(b.GetLocation(),std::invalid_argument);
}

TEST(InvalidBundle, GettingSymbolicNameFromInvalidBundle)
{
    Bundle b;
    EXPECT_THROW(b.GetSymbolicName(),std::invalid_argument);
}

TEST(InvalidBundle, GettingHeadersFromInvalidBundle)
{
    Bundle b;
    EXPECT_THROW(b.GetHeaders(),std::invalid_argument);
}
