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

#include "usThreads.h"

#include "usUtils.h"

#ifdef US_PLATFORM_POSIX
#include <sys/time.h>
#include <errno.h>
#endif

US_BEGIN_NAMESPACE

WaitCondition::WaitCondition()
{
#ifdef US_ENABLE_THREADING_SUPPORT
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
#endif
}

WaitCondition::~WaitCondition()
{
#ifdef US_ENABLE_THREADING_SUPPORT
  #ifdef US_PLATFORM_POSIX
    pthread_cond_destroy(&m_WaitCondition);
  #else
    CloseHandle(m_Semaphore);
    CloseHandle(m_WaitersAreDone);
    DeleteCriticalSection(&m_NumberOfWaitersLock);
  #endif
#endif
}

void WaitCondition::Notify()
{
#ifdef US_ENABLE_THREADING_SUPPORT
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
#endif
}

void WaitCondition::NotifyAll()
{
#ifdef US_ENABLE_THREADING_SUPPORT
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
#endif
}

bool WaitCondition::Wait(MutexType* mutex, unsigned long timeoutMillis)
{
  return Wait(*mutex, timeoutMillis);
}

#ifdef US_ENABLE_THREADING_SUPPORT
bool WaitCondition::Wait(MutexType& mutex, unsigned long timeoutMillis)
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
bool WaitCondition::Wait(MutexType&, unsigned long)
{
  return true;
}
#endif

US_END_NAMESPACE

