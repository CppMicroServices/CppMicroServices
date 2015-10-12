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

  friend class Lock;

  class Lock
  {
  public:

    Lock(const Lock&) = delete;
    Lock& operator=(const Lock&) = delete;

#ifdef US_ENABLE_THREADING_SUPPORT

    // Lock object
    explicit Lock(const MutexLockingStrategy& host)
      : m_Lock(host.m_Mtx)
    {}

    // Lock object
    explicit Lock(const MutexLockingStrategy* host)
      : m_Lock(host->m_Mtx)
    {}

#else
    explicit Lock(const MutexLockingStrategy&) {}
    explicit Lock(const MutexLockingStrategy*) {}
#endif

  private:

    friend class WaitCondition<MutexLockingStrategy>;

#ifdef US_ENABLE_THREADING_SUPPORT
    std::unique_lock<MutexType> m_Lock;
#endif
  };

protected:

#ifdef US_ENABLE_THREADING_SUPPORT
  mutable MutexType m_Mtx;
#endif
};

class NoLockingStrategy {
public:
    typedef void Lock;
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

}


#endif // USTHREADINGMODEL_H
