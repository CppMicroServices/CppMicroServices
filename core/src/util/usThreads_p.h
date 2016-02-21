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


#ifndef USTHREADINGMODEL_H
#define USTHREADINGMODEL_H

#include <usCoreConfig.h>

#include <mutex>

namespace us {

template<class MutexHost>
class WaitCondition;

template<class Mutex = std::mutex>
class MutexLockingStrategy
{
public:

  typedef Mutex MutexType;

  MutexLockingStrategy()
  {}

  MutexLockingStrategy(const MutexLockingStrategy&)
#ifdef US_ENABLE_THREADING_SUPPORT
    : m_Mtx()
#endif
  {}

  friend class UniqueLock;

  class UniqueLock
  {
  public:

    UniqueLock()
    {}

    UniqueLock(const UniqueLock&) = delete;
    UniqueLock& operator=(const UniqueLock&) = delete;

#ifdef US_ENABLE_THREADING_SUPPORT

    UniqueLock(UniqueLock&& o)
      : m_Lock(std::move(o.m_Lock))
    {}

    UniqueLock& operator=(UniqueLock&& o)
    {
      m_Lock = std::move(o.m_Lock);
      return *this;
    }

    // Lock object
    explicit UniqueLock(const MutexLockingStrategy& host)
      : m_Lock(host.m_Mtx)
    {}

    // Lock object
    explicit UniqueLock(const MutexLockingStrategy* host)
      : m_Lock(host->m_Mtx)
    {}

#else
    UniqueLock(UniqueLock&&) {}
    UniqueLock& operator=(UniqueLock&&) {}
    explicit UniqueLock(const MutexLockingStrategy&) {}
    explicit UniqueLock(const MutexLockingStrategy*) {}
#endif

  private:

    friend class WaitCondition<MutexLockingStrategy>;

#ifdef US_ENABLE_THREADING_SUPPORT
    std::unique_lock<MutexType> m_Lock;
#endif
  };

  /**
   * @brief Lock this object.
   *
   * Call this method to lock this object and obtain a lock object
   * which automatically releases the acquired lock when it goes out
   * of scope. E.g.
   *
   * \code
   * auto lock = object->Lock();
   * \endcode
   *
   * @return A lock object.
   */
  UniqueLock Lock() const
  {
    return UniqueLock(this);
  }

protected:

#ifdef US_ENABLE_THREADING_SUPPORT
  mutable MutexType m_Mtx;
#endif
};

class NoLockingStrategy {
public:
    typedef void UniqueLock;
};

template<class MutexHost>
class NoWaitCondition {};

template<
    class LockingStrategy = MutexLockingStrategy<>,
    template<class MutexHost> class WaitConditionStrategy = NoWaitCondition
    >
class MultiThreaded
    : public LockingStrategy
    , public WaitConditionStrategy<LockingStrategy>
{};

template<class T>
class Atomic : private MultiThreaded<>
{
  T m_t;

public:

  T Load() const
  {
    return Lock(), m_t;
  }

  void Store(const T& t)
  {
    Lock(), m_t = t;
  }

  T Exchange(const T& t)
  {
    auto l = Lock(); US_UNUSED(l);
    auto o = m_t;
    m_t = t;
    return o;
  }

};

}


#endif // USTHREADINGMODEL_H
