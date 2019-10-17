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

#include "cppmicroservices/ServiceEvent.h"

#include "gtest/gtest.h"

using namespace cppmicroservices;

template<typename T, typename S = std::string>
void serviceEventOperatorTest(const T type, const S expectedStr)
{
  std::stringstream buffer;
  std::streambuf* backup = std::cout.rdbuf(buffer.rdbuf());
  std::cout << type;
  std::string actStr = buffer.str();
  ASSERT_TRUE(actStr.find(expectedStr) != std::string::npos);
  std::cout.rdbuf(backup);
}

TEST(ServiceEventStreamOperatorTest, serviceEventTypeModified)
{
  serviceEventOperatorTest(ServiceEvent::SERVICE_MODIFIED, "MODIFIED");
}

TEST(ServiceEventStreamOperatorTest, serviceEventTypeModifiedEndMatch)
{
  serviceEventOperatorTest(ServiceEvent::SERVICE_MODIFIED_ENDMATCH, "MODIFIED_ENDMATCH");
}

TEST(ServiceEventStreamOperatorTest, serviceEventTypeDefault)
{
  serviceEventOperatorTest(ServiceEvent::Type(), "unknown service event type");
}

TEST(ServiceEventStreamOperatorTest, serviceEventNone)
{
  ServiceEvent event;
  serviceEventOperatorTest(event, "NONE");
}
