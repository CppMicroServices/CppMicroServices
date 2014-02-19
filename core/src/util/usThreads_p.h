/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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

#ifdef US_ENABLE_THREADING_SUPPORT

  // Atomic compiler intrinsics

  #if defined(US_PLATFORM_APPLE)
    // OSAtomic.h optimizations only used in 10.5 and later
    #include <AvailabilityMacros.h>
    #if MAC_OS_X_VERSION_MAX_ALLOWED >= 1050
      #include <libkern/OSAtomic.h>
      #define US_ATOMIC_OPTIMIZATION_APPLE
    #endif

  #elif defined(__GLIBCPP__) || defined(__GLIBCXX__)

    #if (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 2))
      # include <ext/atomicity.h>
    #else
      # include <bits/atomicity.h>
    #endif
    #define US_ATOMIC_OPTIMIZATION_GNUC

  #endif

  // Mutex support

  #ifdef US_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
      #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
      #define NOMINMAX
    #endif
    #include <windows.h>

    #define US_THREADS_MUTEX(x)           HANDLE (x);
    #define US_THREADS_MUTEX_INIT(x)
    #define US_THREADS_MUTEX_CTOR(x)      : x(::CreateMutex(NULL, FALSE, NULL))
    #define US_THREADS_MUTEX_DELETE(x)    ::CloseHandle (x)
    #define US_THREADS_MUTEX_LOCK(x)      ::WaitForSingleObject (x, INFINITE)
    #define US_THREADS_MUTEX_UNLOCK(x)    ::ReleaseMutex (x)
    #define US_THREADS_LONG               LONG

    #define US_ATOMIC_OPTIMIZATION
    #define US_ATOMIC_INCREMENT(x)        IntType n = InterlockedIncrement(x)
    #define US_ATOMIC_DECREMENT(x)        IntType n = InterlockedDecrement(x)
    #define US_ATOMIC_ASSIGN(l, r)        InterlockedExchange(l, r)

  #elif defined(US_PLATFORM_POSIX)

    #include <pthread.h>

    #define US_THREADS_MUTEX(x)           pthread_mutex_t (x);
    #define US_THREADS_MUTEX_INIT(x)      ::pthread_mutex_init(&x, 0)
    #define US_THREADS_MUTEX_CTOR(x)      : x()
    #define US_THREADS_MUTEX_DELETE(x)    ::pthread_mutex_destroy (&x)
    #define US_THREADS_MUTEX_LOCK(x)      ::pthread_mutex_lock (&x)
    #define US_THREADS_MUTEX_UNLOCK(x)    ::pthread_mutex_unlock (&x)

    #define US_ATOMIC_OPTIMIZATION
    #if defined(US_ATOMIC_OPTIMIZATION_APPLE)
      #if defined (__LP64__) && __LP64__
        #define US_THREADS_LONG           volatile int64_t
        #define US_ATOMIC_INCREMENT(x)    IntType n = OSAtomicIncrement64Barrier(x)
        #define US_ATOMIC_DECREMENT(x)    IntType n = OSAtomicDecrement64Barrier(x)
        #define US_ATOMIC_ASSIGN(l, v)    OSAtomicCompareAndSwap64Barrier(*l, v, l)
      #else
        #define US_THREADS_LONG           volatile int32_t
        #define US_ATOMIC_INCREMENT(x)    IntType n = OSAtomicIncrement32Barrier(x)
        #define US_ATOMIC_DECREMENT(x)    IntType n = OSAtomicDecrement32Barrier(x)
        #define US_ATOMIC_ASSIGN(l, v)    OSAtomicCompareAndSwap32Barrier(*l, v, l)
      #endif
    #elif defined(US_ATOMIC_OPTIMIZATION_GNUC)
      #define US_THREADS_LONG             _Atomic_word
      #define US_ATOMIC_INCREMENT(x)      IntType n = __sync_add_and_fetch(x, 1)
      #define US_ATOMIC_DECREMENT(x)      IntType n = __sync_add_and_fetch(x, -1)
      #define US_ATOMIC_ASSIGN(l, v)      __sync_val_compare_and_swap(l, *l, v)
    #else
      #define US_THREADS_LONG             long
      #undef US_ATOMIC_OPTIMIZATION
      #define US_ATOMIC_INCREMENT(x)      m_AtomicMtx.Lock();  \
                                          IntType n = ++(*x);  \
                                          m_AtomicMtx.Unlock()
      #define US_ATOMIC_DECREMENT(x)      m_AtomicMtx.Lock();  \
                                          IntType n = --(*x);  \
                                          m_AtomicMtx.Unlock()
      #define US_ATOMIC_ASSIGN(l, v)      m_AtomicMtx.Lock();  \
                                          *l = v;              \
                                          m_AtomicMtx.Unlock()
    #endif

  #endif

#else

  // single threaded
  #define US_THREADS_MUTEX(x)
  #define US_THREADS_MUTEX_INIT(x)
  #define US_THREADS_MUTEX_CTOR(x)
  #define US_THREADS_MUTEX_DELETE(x)
  #define US_THREADS_MUTEX_LOCK(x)
  #define US_THREADS_MUTEX_UNLOCK(x)
  #define US_THREADS_LONG int

  #define US_ATOMIC_INCREMENT(x)        IntType n = ++(*x);
  #define US_ATOMIC_DECREMENT(x)        IntType n = --(*x);
  #define US_ATOMIC_ASSIGN(l, r)        *l = r;

#endif


US_BEGIN_NAMESPACE

class Mutex
{
public:

  Mutex() US_THREADS_MUTEX_CTOR(m_Mtx)
  {
    US_THREADS_MUTEX_INIT(m_Mtx);
  }

  ~Mutex()
  {
    US_THREADS_MUTEX_DELETE(m_Mtx);
  }

  void Lock()
  {
    US_THREADS_MUTEX_LOCK(m_Mtx);
  }
  void Unlock()
  {
    US_THREADS_MUTEX_UNLOCK(m_Mtx);
  }

private:

  template<class Host> friend class WaitCondition;

  // Copy-constructor not implemented.
  Mutex(const Mutex &);
  // Copy-assignement operator not implemented.
  Mutex & operator = (const Mutex &);

  US_THREADS_MUTEX(m_Mtx)
};

class MutexLock
{
public:
  typedef Mutex MutexType;

  MutexLock(MutexType& mtx) : m_Mtx(&mtx) { m_Mtx->Lock(); }
  ~MutexLock() { m_Mtx->Unlock(); }

private:
  MutexType* m_Mtx;

  // purposely not implemented
  MutexLock(const MutexLock&);
  MutexLock& operator=(const MutexLock&);
};

class AtomicCounter
{
public:

  typedef US_THREADS_LONG IntType;

  AtomicCounter(int value = 0)
    : m_Counter(value)
  {}

  IntType AtomicIncrement() const
  {
    US_ATOMIC_INCREMENT(&m_Counter);
    return n;
  }

  IntType AtomicIncrement(volatile IntType& lval) const
  {
    US_ATOMIC_INCREMENT(&lval);
    return n;
  }

  IntType AtomicDecrement() const
  {
    US_ATOMIC_DECREMENT(&m_Counter);
    return n;
  }

  IntType AtomicDecrement(volatile IntType& lval) const
  {
    US_ATOMIC_DECREMENT(&lval);
    return n;
  }

  void AtomicAssign(volatile IntType& lval) const
  {
    US_ATOMIC_ASSIGN(&lval, m_Counter);
  }

  void AtomicAssign(volatile IntType& lval, const IntType val) const
  {
    US_ATOMIC_ASSIGN(&lval, val);
  }

  mutable IntType m_Counter;

private:

#if !defined(US_ATOMIC_OPTIMIZATION)
  mutable Mutex m_AtomicMtx;
#endif
};

class MutexLockingStrategy
{
public:

  MutexLockingStrategy()
#ifdef US_ENABLE_THREADING_SUPPORT
    : m_Mtx()
#endif
  {}

  MutexLockingStrategy(const MutexLockingStrategy&)
#ifdef US_ENABLE_THREADING_SUPPORT
    : m_Mtx()
#endif
  {}

  class Lock;
  friend class Lock;

  class Lock
  {
  public:

#ifdef US_ENABLE_THREADING_SUPPORT
    // Lock object
    explicit Lock(const MutexLockingStrategy& host) : m_Host(host)
    {
      m_Host.m_Mtx.Lock();
    }

    // Lock object
    explicit Lock(const MutexLockingStrategy* host) : m_Host(*host)
    {
      m_Host.m_Mtx.Lock();
    }

    // Unlock object
    ~Lock()
    {
      m_Host.m_Mtx.Unlock();
    }
#else
    explicit Lock(const MutexLockingStrategy&) {}
    explicit Lock(const MutexLockingStrategy*) {}
#endif

  private:

    // private by design
    Lock();
    Lock(const Lock&);
    Lock& operator=(const Lock&);
#ifdef US_ENABLE_THREADING_SUPPORT
    const MutexLockingStrategy& m_Host;
#endif
  };

protected:

#ifdef US_ENABLE_THREADING_SUPPORT
  mutable Mutex m_Mtx;
#endif
};

class NoLockingStrategy
{
};

US_END_NAMESPACE

#include <usWaitCondition_p.h>

US_BEGIN_NAMESPACE

template<class LockingStrategy = MutexLockingStrategy,
         template<class Host> class WaitConditionStrategy = NoWaitCondition
        >
class MultiThreaded : public LockingStrategy,
                      public WaitConditionStrategy<MultiThreaded<LockingStrategy, WaitConditionStrategy> >
{
  friend class WaitConditionStrategy<MultiThreaded<LockingStrategy, WaitConditionStrategy> >;
};

US_END_NAMESPACE

#endif // USTHREADINGMODEL_H
