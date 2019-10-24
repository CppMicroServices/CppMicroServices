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

#ifndef __CONCURRENCYUTIL_HPP__
#define __CONCURRENCYUTIL_HPP__

#include <memory>
#include <mutex>
#include <future>
#include <chrono>

namespace cppmicroservices {
namespace scrimpl {

/**
 * Util method to determine if a future object is ready
 */
template<typename T>
bool is_ready(T&& futObj)
{
  return (futObj.wait_for(std::chrono::seconds::zero()) == std::future_status::ready);
}

/**
 * Utility class to help developers avoid mistakes with locking data members
 * in a class. This code is based on the Guarded idiom as described in
 * "Mastering the C++17 STL" by Arthur O'Dwyer
 */
template<class Data>
class Guarded
{
  std::mutex mtx;
  Data data;
  class Handle
  {
    std::unique_lock<std::mutex> lk;
    Data *ptr;
  public:
    Handle(std::unique_lock<std::mutex> lk, Data *p) :
      lk(std::move(lk)),
    ptr(p)
    { }

    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;

    Handle(Handle&& rhs) :
      lk(std::move(rhs.lk)),
    ptr(std::move(rhs.ptr))
    { }

    Handle& operator=(Handle&& rhs)
    {
      lk = std::move(rhs.lk);
      ptr = std::move(rhs.ptr);
    }

    ~Handle() = default;

    Data* operator->() const { return ptr; }
    Data& operator*() const { return *ptr; }
  };
public:
  /**
   * Locks the member mutex and returns an RAII object responsible for
   * unlocking the mutex.
   *
   * \return a RAII style wrapper object used to access the data
   */
  Handle lock()
  {
    std::unique_lock<std::mutex> lock(mtx);
    return Handle{std::move(lock), &data};
  }
};

/**
 * A utility class similar to std::latch except it allows for incrementing the count as well.
 * It is useful when the number of threads entering a block may vary dynamically. This class
 * can be used to synchronize two blocks of code where the first block can be executed by
 * 'n' number of threads simultaneously and the second block needs to wait for all the threads
 * to exit the first block.
 *
 * Example code:
 * function1()
 * {
 *    if(latch.CountUp())
 *    {
 *       // do something critical here
 *       latch.CountDown();
 *    }
 * }
 * function2()
 * {
 *    latch.Wait(); // this blocks the thread until all threads that have exited function1
 *    // do something here.
 * }
 */
class CounterLatch
{
public:
  CounterLatch() : count(0) {}
  CounterLatch(const CounterLatch&) = delete;
  CounterLatch(CounterLatch&&) = delete;
  CounterLatch& operator=(const CounterLatch&) = delete;
  CounterLatch& operator=(CounterLatch&&) = delete;
  ~CounterLatch() = default;

  /**
   * Increments the count of the latch, if current count is not negative
   * \return \c true if the count was incremented, \c false otherwise
   */
  bool CountUp()
  {
    std::lock_guard<std::mutex> lock{mtx};
    if(count >= 0)
    {
      ++count;
      return true;
    }
    return false;
  }

  /**
   * Decrements the count of the latch, releasing all waiting threads if the
   * count reaches zero.
   * If the current count is greater than zero then it is decremented. If
   * the new count is zero then all waiting threads are notified.
   * If the current count equals zero then nothing happens.
   */
  void CountDown()
  {
    std::lock_guard<std::mutex> lock{mtx};
    if(count > 0)
    {
      if(--count == 0)
      {
        // notify waiting threads
        cond.notify_all();
      }
    }
  }

  /**
   * Waits until the counter reaches 0. The value of the counter after this
   * method returns is invalid (negative). This method is designed for a
   * one-time use only.
   *
   * \throws std::runtime_error if the current count is negative.
   */
  void Wait()
  {
    std::unique_lock<std::mutex> lock{mtx};
    if(count < 0)
    {
      throw std::runtime_error("CounterLatch is in invalid state.");
    }
    cond.wait(lock, [&]() { return count == 0; });
    count = std::numeric_limits<long>::min(); // makes the latch unusable for other threads
  }

  /**
   * Returns the current count of the latch
   */
  long GetCount()
  {
    std::unique_lock<std::mutex> lock{mtx};
    return count;
  }

private:
  std::mutex mtx; ///< mutex to protect access to the counter
  std::condition_variable cond; ///< used to notify the waiting thread
  long count{0}; ///< latch counter
};
}
}

#endif /* __CONCURRENCYUTIL_HPP__ */
