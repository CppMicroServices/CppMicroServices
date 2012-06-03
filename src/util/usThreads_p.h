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

#include <usConfig.h>

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
  #define US_THREADS_LONG

#endif

#ifndef US_DEFAULT_MUTEX
  #define US_DEFAULT_MUTEX US_PREPEND_NAMESPACE(Mutex)
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

  friend class WaitCondition;

  // Copy-constructor not implemented.
  Mutex(const Mutex &);
  // Copy-assignement operator not implemented.
  Mutex & operator = (const Mutex &);

  US_THREADS_MUTEX(m_Mtx)
};

template<class MutexPolicy = US_DEFAULT_MUTEX>
class MutexLock
{
public:
  typedef MutexPolicy MutexType;

  MutexLock(MutexType& mtx) : m_Mtx(&mtx) { m_Mtx->Lock(); }
  ~MutexLock() { m_Mtx->Unlock(); }

private:
  MutexType* m_Mtx;

  // purposely not implemented
  MutexLock(const MutexLock&);
  MutexLock& operator=(const MutexLock&);
};

typedef MutexLock<> DefaultMutexLock;


/**
 * \brief A thread synchronization object used to suspend execution until some
 * condition on shared data is met.
 *
 * A thread calls Wait() to suspend its execution until the condition is
 * met. Each call to Notify() from an executing thread will then cause a single
 * waiting thread to be released.  A call to Notify() means, "signal
 * that the condition is true."  NotifyAll() releases all threads waiting on
 * the condition variable.
 *
 * The WaitCondition implementation is consistent with the standard
 * definition and use of condition variables in pthreads and other common
 * thread libraries.
 *
 * IMPORTANT: A condition variable always requires an associated mutex
 * object. The mutex object is used to avoid a dangerous race condition when
 * Wait() and Notify() are called simultaneously from two different
 * threads.
 *
 * On systems using pthreads, this implementation abstracts the
 * standard calls to the pthread condition variable.  On Win32
 * systems, there is no system provided condition variable.  This
 * class implements a condition variable using a critical section, a
 * semphore, an event and a number of counters.  The implementation is
 * almost an extract translation of the implementation presented by
 * Douglas C Schmidt and Irfan Pyarali in "Strategies for Implementing
 * POSIX Condition Variables on Win32". This article can be found at
 * http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
 *
 */
class US_EXPORT WaitCondition
{
public:

  typedef US_DEFAULT_MUTEX MutexType;

  WaitCondition();
  ~WaitCondition();

  /** Suspend execution of this thread until the condition is signaled. The
   *  argument is a SimpleMutex object that must be locked prior to calling
   *  this method.  */
  bool Wait(MutexType& mutex, unsigned long time = 0);

  bool Wait(MutexType* mutex, unsigned long time = 0);

  /** Notify that the condition is true and release one waiting thread */
  void Notify();

  /** Notify that the condition is true and release all waiting threads */
  void NotifyAll();

private:

  // purposely not implemented
  WaitCondition(const WaitCondition& other);
  const WaitCondition& operator=(const WaitCondition&);

#ifdef US_ENABLE_THREADING_SUPPORT
  #ifdef US_PLATFORM_POSIX
  pthread_cond_t m_WaitCondition;
  #else

  int m_NumberOfWaiters;                   // number of waiting threads
  CRITICAL_SECTION m_NumberOfWaitersLock;  // Serialize access to
  // m_NumberOfWaiters

  HANDLE m_Semaphore;                      // Semaphore to queue threads
  HANDLE m_WaitersAreDone;                 // Auto-reset event used by the
                                           // broadcast/signal thread to
                                           // wait for all the waiting
                                           // threads to wake up and
                                           // release the semaphore

  std::size_t m_WasNotifyAll;              // Keeps track of whether we
                                           // were broadcasting or signaling
  #endif

#endif
};

US_END_NAMESPACE

#ifdef US_ENABLE_THREADING_SUPPORT

US_BEGIN_NAMESPACE

template<class Host, class MutexPolicy = US_DEFAULT_MUTEX>
class MultiThreaded
{
  mutable MutexPolicy m_Mtx;
  WaitCondition m_Cond;

  #if !defined(US_ATOMIC_OPTIMIZATION)
  mutable MutexPolicy m_AtomicMtx;
  #endif

public:

  MultiThreaded() : m_Mtx(), m_Cond() {}
  MultiThreaded(const MultiThreaded&) : m_Mtx(), m_Cond() {}
  virtual ~MultiThreaded() {}

  class Lock;
  friend class Lock;

  class Lock
  {
  public:

    // Lock object
    explicit Lock(const MultiThreaded& host) : m_Host(host)
    {
      m_Host.m_Mtx.Lock();
    }

    // Lock object
    explicit Lock(const MultiThreaded* host) : m_Host(*host)
    {
      m_Host.m_Mtx.Lock();
    }

    // Unlock object
    ~Lock()
    {
      m_Host.m_Mtx.Unlock();
    }

  private:

    // private by design
    Lock();
    Lock(const Lock&);
    Lock& operator=(const Lock&);
    const MultiThreaded& m_Host;

  };

  typedef volatile Host VolatileType;
  typedef US_THREADS_LONG IntType;

  bool Wait(unsigned long timeoutMillis = 0)
  {
    return m_Cond.Wait(m_Mtx, timeoutMillis);
  }

  void Notify()
  {
    m_Cond.Notify();
  }

  void NotifyAll()
  {
    m_Cond.NotifyAll();
  }

  IntType AtomicIncrement(volatile IntType& lval) const
  {
    US_ATOMIC_INCREMENT(&lval);
    return n;
  }

  IntType AtomicDecrement(volatile IntType& lval) const
  {
    US_ATOMIC_DECREMENT(&lval);
    return n;
  }

  void AtomicAssign(volatile IntType& lval, const IntType val) const
  {
    US_ATOMIC_ASSIGN(&lval, val);
  }

};

US_END_NAMESPACE

#endif


US_BEGIN_NAMESPACE

template<class Host, class MutexPolicy = US_DEFAULT_MUTEX>
class SingleThreaded
{
public:

  virtual ~SingleThreaded() {}

  // Dummy Lock class
  struct Lock
  {
    Lock() {}
    explicit Lock(const SingleThreaded&) {}
    explicit Lock(const SingleThreaded*) {}
  };

  typedef Host VolatileType;
  typedef int IntType;

  bool Wait(unsigned long = 0)
  { return false; }

  void Notify() {}

  void NotifyAll() {}

  static IntType AtomicAdd(volatile IntType& lval, const IntType val)
  { return lval += val; }

  static IntType AtomicSubtract(volatile IntType& lval, const IntType val)
  { return lval -= val; }

  static IntType AtomicMultiply(volatile IntType& lval, const IntType val)
  { return lval *= val; }

  static IntType AtomicDivide(volatile IntType& lval, const IntType val)
  { return lval /= val; }

  static IntType AtomicIncrement(volatile IntType& lval)
  { return ++lval; }

  static IntType AtomicDecrement(volatile IntType& lval)
  { return --lval; }

  static void AtomicAssign(volatile IntType & lval, const IntType val)
  { lval = val; }

  static void AtomicAssign(IntType & lval, volatile IntType & val)
  { lval = val; }

};

US_END_NAMESPACE

#endif // USTHREADINGMODEL_H
