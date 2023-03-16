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

#include "cppmicroservices/ShrinkableMap.h"

#include "TestUtils.h"
#include "TestingConfig.h"
#include "gtest/gtest.h"

namespace cppmicroservices
{

    // Fake a ServiceHooks class so we can create
    // ShrinkableMap instances
    class ServiceHooks
    {

      public:
        template <class K, class V>
        static ShrinkableMap<K, V>
        MakeMap(std::map<K, V>& m)
        {
            return ShrinkableMap<K, V>(m);
        }
    };
} // namespace cppmicroservices

using namespace cppmicroservices;

TEST(ShrinkableMapTest, testShrinkableMapOperations)
{
    ShrinkableMap<int, std::string>::container_type m {
        {1,   "one"},
        {2,   "two"},
        {3, "three"}
    };

    auto shrinkable = ServiceHooks::MakeMap(m);

    // Original size
    ASSERT_EQ(m.size(), 3);
    // Equal size
    ASSERT_EQ(m.size(), shrinkable.size());
    // At access
    ASSERT_EQ(shrinkable.at(1), "one");

    shrinkable.erase(shrinkable.find(1));
    // New size
    ASSERT_EQ(m.size(), 2);
    EXPECT_THROW(shrinkable.at(1), std::out_of_range);
    // back() access
    ASSERT_EQ(shrinkable.at(2), "two");
}
