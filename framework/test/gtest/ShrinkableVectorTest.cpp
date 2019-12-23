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

#include "cppmicroservices/ShrinkableVector.h"

#include "gtest/gtest.h"

using namespace cppmicroservices;

// Fake a BundleHooks class so we can create
// ShrinkableVector instances
namespace cppmicroservices 
{
class BundleHooks
{

public:
  template<class E>
  static ShrinkableVector<E> MakeVector(std::vector<E>& c)
  {
    return ShrinkableVector<E>(c);
  }
};
}

TEST(ShrinkableVectorTest, ShrinkableVector)
{
  ShrinkableVector<int32_t>::container_type vec({1, 2, 3});
  auto shrinkable = BundleHooks::MakeVector(vec);

  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec.size(), shrinkable.size());
  EXPECT_EQ(shrinkable.at(0), 1);
  EXPECT_EQ(shrinkable.back(), 3);

  shrinkable.pop_back();
  EXPECT_EQ(shrinkable.back(), 2);
  EXPECT_THROW(shrinkable.at(3), std::out_of_range);
}
