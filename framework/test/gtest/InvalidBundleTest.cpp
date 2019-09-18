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
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleResource.h"

#include "gtest/gtest.h"
#include <stdexcept>

using namespace cppmicroservices;

TEST(InvalidBundle, GetBundleIdFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.GetBundleId(), std::invalid_argument);
}

TEST(InvalidBundle, GetBundleLocationFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.GetLocation(), std::invalid_argument);
}

TEST(InvalidBundle, GetSymbolicNameFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.GetSymbolicName(), std::invalid_argument);
}

TEST(InvalidBundle, GetHeadersFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.GetHeaders(), std::invalid_argument);
}

TEST(InvalidBundle, GetPropertiesFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.GetProperties(), std::invalid_argument);
}

TEST(InvalidBundle, StartFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.Start(), std::invalid_argument);
}

TEST(InvalidBundle, StartWithOptionsFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.Start(32), std::invalid_argument);
}

TEST(InvalidBundle, StopFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.Stop(32), std::invalid_argument);
}

TEST(InvalidBundle, UninstallFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.Uninstall(), std::invalid_argument);
}

TEST(InvalidBundle, GetBundleContextFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.GetBundleContext(), std::invalid_argument);
}

TEST(InvalidBundle, GetPropertyFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.GetProperty(""), std::invalid_argument);
}

TEST(InvalidBundle, GetPropertyKeysFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.GetPropertyKeys(), std::invalid_argument);
}

TEST(InvalidBundle, GetRegisteredServicesFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.GetRegisteredServices(), std::invalid_argument);
}

TEST(InvalidBundle, GetServicesInUseFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.GetServicesInUse(), std::invalid_argument);
}

TEST(InvalidBundle, GetResourceFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.GetResource(""), std::invalid_argument);
}

TEST(InvalidBundle, FindResourcesFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.FindResources("", "", true), std::invalid_argument);
}

TEST(InvalidBundle, GetLastModifiedFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.GetLastModified(), std::invalid_argument);
}

TEST(InvalidBundle, GetBundleStateFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.GetState(), std::invalid_argument);
}

TEST(InvalidBundle, GetBundleVersionFromInvalidBundle)
{
  Bundle b;
  EXPECT_THROW(b.GetVersion(), std::invalid_argument);
}
