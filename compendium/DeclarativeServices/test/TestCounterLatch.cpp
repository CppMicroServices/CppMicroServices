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

#include "gtest/gtest.h"
#include "../src/manager/ConcurrencyUtil.hpp"
#include "ConcurrencyTestUtil.hpp"

namespace cppmicroservices{
namespace scrimpl {

TEST(CounterLatchTest, TestInitialState)
{
  CounterLatch latch;
  EXPECT_EQ(latch.GetCount(), 0) << "Initial count of the latch must be zero";
}

TEST(CounterLatchTest, TestCountUp)
{
  CounterLatch latch;
  for(int i =0; i<50; i++)
  {
    latch.CountUp();
    EXPECT_EQ(latch.GetCount(), i+1) << "Latch count must be equal to the number of times CountUp is called";
  }
}

TEST(CounterLatchTest, TestCountUpConcurrent)
{
  CounterLatch latch;
  std::function<bool()> func = [&latch]() { return latch.CountUp(); };
  auto results = ConcurrentInvoke(func);
  EXPECT_TRUE(latch.GetCount() > 0) << "Latch count must be greater than zero after concurrent calls to CountUp";
}

TEST(CounterLatchTest, TestCountDown_Initial)
{
  CounterLatch latch;
  EXPECT_EQ(latch.GetCount(), 0) << "Initial count of the latch must be zero";
  for(int i =0; i<50; i++)
  {
    latch.CountDown();
    EXPECT_EQ(latch.GetCount(), 0) << "Latch count must stay at zero if CountDown is called when count is zero";
  }
}

TEST(CounterLatchTest, TestCountDown_AfterCountUp)
{
  CounterLatch latch;
  EXPECT_EQ(latch.GetCount(), 0) << "Initial count of the latch must be zero";
  int iterCount = 50;
  for(int i =0; i<iterCount; i++)
  {
    latch.CountUp();

  }
  EXPECT_EQ(latch.GetCount(), iterCount) << "Latch count must be equal to number of calls to CountUp";
  for(int i =0; i<iterCount; i++)
  {
    latch.CountDown();
  }
  EXPECT_EQ(latch.GetCount(), 0) << "Latch count must be zero after calls to CountDown";
}

TEST(CounterLatchTest, TestWait)
{
  CounterLatch latch;
  EXPECT_EQ(latch.GetCount(), 0) << "Initial count of the latch must be zero";
  EXPECT_NO_THROW(latch.Wait()) << "First call to Wait must not throw";
  EXPECT_TRUE(latch.GetCount() < 0) << "latch count must negative after call to Wait";
  // subsequent calls to wait will result in an exception
  EXPECT_THROW(latch.Wait(), std::runtime_error) << "Second call to Wait must throw";
}

TEST(CounterLatchTest, TestWaitAfterCountUp)
{
  CounterLatch latch;
  EXPECT_EQ(latch.GetCount(), 0);
  std::function<bool()> func = [&latch]() { return latch.CountUp(); };
  auto results = ConcurrentInvoke(func);
  EXPECT_TRUE(latch.GetCount() > 0) << "latch count must be positive after calls to count up";
  auto fut = std::async(std::launch::async, [&latch]() { latch.Wait(); });
  EXPECT_FALSE(is_ready(fut)) << "The call to Wait must not return yet";
  while(latch.GetCount() > 0)
  {
    latch.CountDown();
  }
  EXPECT_TRUE(latch.GetCount() < 1) << "latch count could be 0 if the waiting thread has not woke up, negative if the waiting thread woke up";
  EXPECT_NO_THROW(fut.get()) << "The call to Wait must have returned";
  EXPECT_TRUE(latch.GetCount() < 0);
}
}
}
