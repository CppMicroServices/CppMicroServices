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

#ifndef USWAITCONDITION_P_H
#define USWAITCONDITION_P_H

#include "usCoreConfig.h"
#include "usLog_p.h"
#include "usUtils_p.h"
#include "usThreads_p.h"

#ifdef US_PLATFORM_POSIX
#include <sys/time.h>
#include <errno.h>
#endif

US_BEGIN_NAMESPACE

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
template<class MutexHost>
class WaitCondition
{
public:

  WaitCondition();
  ~WaitCondition();

  bool Wait(unsigned long time = 0);

  /** Notify that the condition is true and release one waiting thread */
  void Notify();

  /** Notify that the condition is true and release all waiting threads */
  void NotifyAll();

private:

  // purposely not implemented
  WaitCondition(const WaitCondition& other);
  const WaitCondition& operator=(const WaitCondition&);

#ifdef US_ENABLE_THREADING_SUPPORT

  /** Suspend execution of this thread until the condition is signaled. The
   *  argument is a SimpleMutex object that must be locked prior to calling
   *  this method.  */
  bool Wait(Mutex& mutex, unsigned long time = 0);

  bool Wait(Mutex* mutex, unsigned long time = 0);

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

#endif // US_ENABLE_THREADING_SUPPORT
};

template<class MutexHost>
class NoWaitCondition
{
public:
  NoWaitCondition() {}
private:
  // purposely not implemented
  NoWaitCondition(const NoWaitCondition& other);
  const NoWaitCondition& operator=(const NoWaitCondition&);
};

// ------------------------------------------------------------------------
// WaitCondition implementation
// ------------------------------------------------------------------------

#ifdef US_ENABLE_THREADING_SUPPORT
template<class MutexHost>
WaitCondition<MutexHost>::WaitCondition()
{
  #ifdef US_PLATFORM_POSIX
    pthread_cond_init(&m_WaitCondition, 0);
  #else
    m_NumberOfWaiters = 0;
    m_WasNotifyAll = 0;
    m_Semaphore = CreateSemaphore(0,            // no security
                                  0,            // initial value
                                  0x7fffffff,   // max count
                                  0);           // unnamed
    InitializeCriticalSection(&m_NumberOfWaitersLock);
    m_WaitersAreDone = CreateEvent(0,           // no security
                                   FALSE,       // auto-reset
                                   FALSE,       // non-signaled initially
                                   0 );         // unnamed
  #endif
}

template<class MutexHost>
WaitCondition<MutexHost>::~WaitCondition()
{
  #ifdef US_PLATFORM_POSIX
    pthread_cond_destroy(&m_WaitCondition);
  #else
    CloseHandle(m_Semaphore);
    CloseHandle(m_WaitersAreDone);
    DeleteCriticalSection(&m_NumberOfWaitersLock);
  #endif
}

template<class MutexHost>
bool WaitCondition<MutexHost>::Wait(unsigned long time)
{
  return this->Wait(static_cast<MutexHost*>(this)->m_Mtx, time);
}

template<class MutexHost>
void WaitCondition<MutexHost>::Notify()
{
  #ifdef US_PLATFORM_POSIX
    pthread_cond_signal(&m_WaitCondition);
  #else
    EnterCriticalSection(&m_NumberOfWaitersLock);
    int haveWaiters = m_NumberOfWaiters > 0;
    LeaveCriticalSection(&m_NumberOfWaitersLock);

    // if there were not any waiters, then this is a no-op
    if (haveWaiters)
    {
      ReleaseSemaphore(m_Semaphore, 1, 0);
    }
  #endif
}

template<class MutexHost>
void WaitCondition<MutexHost>::NotifyAll()
{
  #ifdef US_PLATFORM_POSIX
    pthread_cond_broadcast(&m_WaitCondition);
  #else
    // This is needed to ensure that m_NumberOfWaiters and m_WasNotifyAll are
    // consistent
    EnterCriticalSection(&m_NumberOfWaitersLock);
    int haveWaiters = 0;

    if (m_NumberOfWaiters > 0)
    {
      // We are broadcasting, even if there is just one waiter...
      // Record that we are broadcasting, which helps optimize Notify()
      // for the non-broadcast case
      m_WasNotifyAll = 1;
      haveWaiters = 1;
    }

    if (haveWaiters)
    {
      // Wake up all waiters atomically
      ReleaseSemaphore(m_Semaphore, m_NumberOfWaiters, 0);

      LeaveCriticalSection(&m_NumberOfWaitersLock);

      // Wait for all the awakened threads to acquire the counting
      // semaphore
      WaitForSingleObject(m_WaitersAreDone, INFINITE);
      // This assignment is ok, even without the m_NumberOfWaitersLock held
      // because no other waiter threads can wake up to access it.
      m_WasNotifyAll = 0;
    }
    else
    {
      LeaveCriticalSection(&m_NumberOfWaitersLock);
    }
  #endif
}

template<class MutexHost>
bool WaitCondition<MutexHost>::Wait(Mutex* mutex, unsigned long timeoutMillis)
{
  return Wait(*mutex, timeoutMillis);
}

template<class MutexHost>
bool WaitCondition<MutexHost>::Wait(Mutex& mutex, unsigned long timeoutMillis)
{
  #ifdef US_PLATFORM_POSIX
    struct timespec ts, * pts = 0;
    if (timeoutMillis)
    {
      pts = &ts;
      struct timeval tv;
      int error = gettimeofday(&tv, NULL);
      if (error)
      {
        US_ERROR << "gettimeofday error: " << GetLastErrorStr();
        return false;
      }
      ts.tv_sec = tv.tv_sec;
      ts.tv_nsec = tv.tv_usec * 1000;
      ts.tv_sec += timeoutMillis / 1000;
      ts.tv_nsec += (timeoutMillis % 1000) * 1000000;
      ts.tv_sec += ts.tv_nsec / 1000000000;
      ts.tv_nsec = ts.tv_nsec % 1000000000;
    }

    if (pts)
    {
      int error = pthread_cond_timedwait(&m_WaitCondition, &mutex.m_Mtx, pts);
      if (error == 0)
      {
        return true;
      }
      else
      {
        if (error != ETIMEDOUT)
        {
          US_ERROR << "pthread_cond_timedwait error: " << GetLastErrorStr();
        }
        return false;
      }
    }
    else
    {
      int error = pthread_cond_wait(&m_WaitCondition, &mutex.m_Mtx);
      if (error)
      {
        US_ERROR << "pthread_cond_wait error: " << GetLastErrorStr();
        return false;
      }
      return true;
    }

  #else

    // Avoid race conditions
    EnterCriticalSection(&m_NumberOfWaitersLock);
    m_NumberOfWaiters++;
    LeaveCriticalSection(&m_NumberOfWaitersLock);

    DWORD dw;
    bool result = true;

    // This call atomically releases the mutex and waits on the
    // semaphore until signaled
    dw = SignalObjectAndWait(mutex.m_Mtx, m_Semaphore, timeoutMillis ? timeoutMillis : INFINITE, FALSE);
    if (dw == WAIT_TIMEOUT)
    {
      result = false;
    }
    else if (dw == WAIT_FAILED)
    {
      result = false;
      US_ERROR << "SignalObjectAndWait failed: " << GetLastErrorStr();
    }

    // Reacquire lock to avoid race conditions
    EnterCriticalSection(&m_NumberOfWaitersLock);

    // We're no longer waiting....
    m_NumberOfWaiters--;

    // Check to see if we're the last waiter after the broadcast
    int lastWaiter = m_WasNotifyAll && m_NumberOfWaiters == 0;

    LeaveCriticalSection(&m_NumberOfWaitersLock);

    // If we're the last waiter thread during this particular broadcast
    // then let the other threads proceed
    if (lastWaiter)
    {
      // This call atomically signals the m_WaitersAreDone event and waits
      // until it can acquire the external mutex.  This is required to
      // ensure fairness
      dw = SignalObjectAndWait(m_WaitersAreDone, mutex.m_Mtx,
                          INFINITE, FALSE);
      if (result && dw == WAIT_FAILED)
      {
        result = false;
        US_ERROR << "SignalObjectAndWait failed: " << GetLastErrorStr();
      }
    }
    else
    {
      // Always regain the external mutex since that's the guarentee we
      // give to our callers
      dw = WaitForSingleObject(mutex.m_Mtx, INFINITE);
      if (result && dw == WAIT_FAILED)
      {
        result = false;
        US_ERROR << "SignalObjectAndWait failed: " << GetLastErrorStr();
      }
    }

    return result;
  #endif
}

#else

template<class MutexHost>
WaitCondition<MutexHost>::WaitCondition() {}

template<class MutexHost>
WaitCondition<MutexHost>::~WaitCondition() {}

template<class MutexHost>
bool WaitCondition<MutexHost>::Wait(unsigned long)
{
  return true;
}

template<class MutexHost>
void WaitCondition<MutexHost>::Notify() {}

template<class MutexHost>
void WaitCondition<MutexHost>::NotifyAll() {}

#endif

US_END_NAMESPACE

#endif // USWAITCONDITION_P_H
