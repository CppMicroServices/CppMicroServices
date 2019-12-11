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

using namespace cppmicroservices;

void streamOperatorTest(Bundle::State bundleState, std::string expectedStr)
{
  std::stringstream buffer;
  std::streambuf* backup = std::cout.rdbuf(buffer.rdbuf());
  std::cout << bundleState;
  std::string actStr = buffer.str();
  ASSERT_TRUE(actStr.find(expectedStr) != std::string::npos);
  std::cout.rdbuf(backup);
}

TEST(BundleStreamOperatorTest, bundleStateUninstalled)
{
  streamOperatorTest(Bundle::STATE_UNINSTALLED, "UNINSTALLED");
}

TEST(BundleStreamOperatorTest, bundleStateStarting)
{
  streamOperatorTest(Bundle::STATE_STARTING, "STARTING");
}

TEST(BundleStreamOperatorTest, bundleStateStopping)
{
  streamOperatorTest(Bundle::STATE_STOPPING, "STOPPING");
}

TEST(BundleStreamOperatorTest, bundleStateDefault)
{
  streamOperatorTest(Bundle::State(), std::string{});
}
