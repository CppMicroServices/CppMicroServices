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

#ifndef CPPMICROSERVICES_THREADS_H
#define CPPMICROSERVICES_THREADS_H

#include "cppmicroservices/FrameworkConfig.h"

#include <atomic>
#include <memory>
#include <mutex>

namespace cppmicroservices
{

    namespace detail
    {

        template <class MutexHost>
        class WaitCondition;

        template <class Mutex = std::mutex>
        class MutexLockingStrategy
        {
          public:
            using MutexType = Mutex;

            MutexLockingStrategy() = default;

            MutexLockingStrategy(MutexLockingStrategy const&)
#ifdef US_ENABLE_THREADING_SUPPORT
                : m_Mtx()
#endif
            {
            }

            friend class UniqueLock;

            class UniqueLock
            {
              public:
                UniqueLock() = default;

                UniqueLock(UniqueLock const&) = delete;
                UniqueLock& operator=(UniqueLock const&) = delete;

#ifdef US_ENABLE_THREADING_SUPPORT

                UniqueLock(UniqueLock&& o) noexcept : m_Lock(std::move(o.m_Lock)) {}

                UniqueLock&
                operator=(UniqueLock&& o) noexcept
                {
                    m_Lock = std::move(o.m_Lock);
                    return *this;
                }

                // Lock object
                explicit UniqueLock(MutexLockingStrategy const& host) : m_Lock(host.m_Mtx) {}

                // Lock object
                explicit UniqueLock(MutexLockingStrategy const* host) : m_Lock(host->m_Mtx) {}

                UniqueLock(MutexLockingStrategy const& host, std::defer_lock_t d) : m_Lock(host.m_Mtx, d) {}

                void
                Lock()
                {
                    m_Lock.lock();
                }

                void
                UnLock()
                {
                    m_Lock.unlock();
                }

                template <typename Rep, typename Period>
                bool
                TryLockFor(std::chrono::duration<Rep, Period> const& duration)
                {
                    return m_Lock.try_lock_for(duration);
                }

#else
                UniqueLock(UniqueLock&&) {}
                UniqueLock&
                operator=(UniqueLock&&)
                {
                    return *this;
                }
                explicit UniqueLock(MutexLockingStrategy const&) {}
                explicit UniqueLock(MutexLockingStrategy const*) {}
                void
                Lock()
                {
                }
                void
                UnLock()
                {
                }
                template <typename Rep, typename Period>
                bool
                TryLockFor(std::chrono::duration<Rep, Period> const&)
                {
                    return true;
                }
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
            UniqueLock
            Lock() const
            {
                return UniqueLock(this);
            }

            UniqueLock
            DeferLock() const
            {
                return UniqueLock(this);
            }

          protected:
#ifdef US_ENABLE_THREADING_SUPPORT
            mutable MutexType m_Mtx;
#endif
        };

        class NoLockingStrategy
        {
          public:
            using UniqueLock = void;
        };

        template <class MutexHost>
        class NoWaitCondition
        {
        };

        template <class LockingStrategy = MutexLockingStrategy<>,
                  template <class MutexHost> class WaitConditionStrategy = NoWaitCondition>
        class MultiThreaded
            : public LockingStrategy
            , public WaitConditionStrategy<LockingStrategy>
        {
        };

        template <class T>
        class Atomic : private MultiThreaded<>
        {
            T m_t;

          public:
            template <class... Args>
            Atomic(Args&&... args) : m_t { std::forward<Args>(args)... }
            {
            }

            T
            Load() const
            {
                return Lock(), m_t;
            }

            void
            Store(T const& t)
            {
                Lock(), m_t = t;
            }

            T
            Exchange(T const& t)
            {
                auto l = Lock();
                US_UNUSED(l);
                auto o = m_t;
                m_t = t;
                return o;
            }

            bool
            CompareExchange(T& expected, T const& desired)
            {
                auto l = Lock();
                US_UNUSED(l);
                if (expected == m_t)
                {
                    m_t = desired;
                    return true;
                }
                expected = m_t;
                return false;
            }
        };


// Check for C++17 atomic shared_ptr support
#if defined(__cpp_lib_atomic_shared_ptr) && __cpp_lib_atomic_shared_ptr >= 201711L
        template <typename T>
        class Atomic<std::shared_ptr<T>> : public std::atomic<std::shared_ptr<T>>
        {
        public:
            using std::atomic<std::shared_ptr<T>>::atomic; // inherit constructors
        };
#else
        template <class T>
        class Atomic<std::shared_ptr<T>>
        {

            std::shared_ptr<T> m_t;

          public:
            std::shared_ptr<T>
            Load() const
            {
                return std::atomic_load(&m_t);
            }

            void
            Store(std::shared_ptr<T> const& t)
            {
                std::atomic_store(&m_t, t);
            }

            std::shared_ptr<T>
            Exchange(std::shared_ptr<T> const& t)
            {
                return std::atomic_exchange(&m_t, t);
            }

            bool
            CompareExchange(std::shared_ptr<T>& expected, std::shared_ptr<T> const& desired)
            {
                return std::atomic_compare_exchange_strong(&m_t, &expected, desired);
            }
        };
#endif

    } // namespace detail

} // namespace cppmicroservices

#endif // CPPMICROSERVICES_THREADS_H
