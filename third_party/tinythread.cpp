/* -*- mode: c++; tab-width: 2; indent-tabs-mode: nil; -*-
Copyright (c) 2010-2012 Marcus Geelnard

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#include <exception>
#include "tinythread.h"

#if defined(_TTHREAD_POSIX_)
  #include <unistd.h>
  #include <map>
#elif defined(_TTHREAD_WIN32_)
  #include <process.h>
#endif


namespace tthread {

//------------------------------------------------------------------------------
// condition_variable
//------------------------------------------------------------------------------
// NOTE 1: The Win32 implementation of the condition_variable class is based on
// the corresponding implementation in GLFW, which in turn is based on a
// description by Douglas C. Schmidt and Irfan Pyarali:
// http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
//
// NOTE 2: Windows Vista actually has native support for condition variables
// (InitializeConditionVariable, WakeConditionVariable, etc), but we want to
// be portable with pre-Vista Windows versions, so TinyThread++ does not use
// Vista condition variables.
//------------------------------------------------------------------------------

#if defined(_TTHREAD_WIN32_)
  #define _CONDITION_EVENT_ONE 0
  #define _CONDITION_EVENT_ALL 1
#endif

#if defined(_TTHREAD_WIN32_)
condition_variable::condition_variable() : mWaitersCount(0)
{
  mEvents[_CONDITION_EVENT_ONE] = CreateEvent(NULL, FALSE, FALSE, NULL);
  mEvents[_CONDITION_EVENT_ALL] = CreateEvent(NULL, TRUE, FALSE, NULL);
  InitializeCriticalSection(&mWaitersCountLock);
}
#endif

#if defined(_TTHREAD_WIN32_)
condition_variable::~condition_variable()
{
  CloseHandle(mEvents[_CONDITION_EVENT_ONE]);
  CloseHandle(mEvents[_CONDITION_EVENT_ALL]);
  DeleteCriticalSection(&mWaitersCountLock);
}
#endif

#if defined(_TTHREAD_WIN32_)
void condition_variable::_wait()
{
  // Wait for either event to become signaled due to notify_one() or
  // notify_all() being called
  int result = WaitForMultipleObjects(2, mEvents, FALSE, INFINITE);

  // Check if we are the last waiter
  EnterCriticalSection(&mWaitersCountLock);
  -- mWaitersCount;
  bool lastWaiter = (result == (WAIT_OBJECT_0 + _CONDITION_EVENT_ALL)) &&
                    (mWaitersCount == 0);
  LeaveCriticalSection(&mWaitersCountLock);

  // If we are the last waiter to be notified to stop waiting, reset the event
  if(lastWaiter)
    ResetEvent(mEvents[_CONDITION_EVENT_ALL]);
}
#endif

#if defined(_TTHREAD_WIN32_)
void condition_variable::notify_one()
{
  // Are there any waiters?
  EnterCriticalSection(&mWaitersCountLock);
  bool haveWaiters = (mWaitersCount > 0);
  LeaveCriticalSection(&mWaitersCountLock);

  // If we have any waiting threads, send them a signal
  if(haveWaiters)
    SetEvent(mEvents[_CONDITION_EVENT_ONE]);
}
#endif

#if defined(_TTHREAD_WIN32_)
void condition_variable::notify_all()
{
  // Are there any waiters?
  EnterCriticalSection(&mWaitersCountLock);
  bool haveWaiters = (mWaitersCount > 0);
  LeaveCriticalSection(&mWaitersCountLock);

  // If we have any waiting threads, send them a signal
  if(haveWaiters)
    SetEvent(mEvents[_CONDITION_EVENT_ALL]);
}
#endif


//------------------------------------------------------------------------------
// POSIX pthread_t to unique thread::id mapping logic.
// Note: Here we use a global thread safe std::map to convert instances of
// pthread_t to small thread identifier numbers (unique within one process).
// This method should be portable across different POSIX implementations.
//------------------------------------------------------------------------------

#if defined(_TTHREAD_POSIX_)
static thread::id _pthread_t_to_ID(const pthread_t &aHandle)
{
  static mutex idMapLock;
  static std::map<pthread_t, unsigned long int> idMap;
  static unsigned long int idCount(1);

  lock_guard<mutex> guard(idMapLock);
  if(idMap.find(aHandle) == idMap.end())
    idMap[aHandle] = idCount ++;
  return thread::id(idMap[aHandle]);
}
#endif // _TTHREAD_POSIX_


//------------------------------------------------------------------------------
// thread
//------------------------------------------------------------------------------

/// Information shared between the thread wrapper and the thread object.
class _thread_wrapper {
  public:
    _thread_wrapper(void (*aFunction)(void *), void * aArg) :
      mFunction(aFunction),
      mArg(aArg),
      mRefCount(2)      // Upon creation the object is referenced by two
                        // instances: the thread object and the thread wrapper
    {
    }

    inline void run()
    {
      mFunction(mArg);
    }

    inline bool joinable() const
    {
      return mRefCount > 1;
    }

    inline bool release()
    {
      return !(--mRefCount);
    }

  private:
    void (*mFunction)(void *);  // Pointer to the function to be executed
    void * mArg;                // Function argument for the thread function
    atomic_int mRefCount;       // Reference count
};

// Thread wrapper function.
#if defined(_TTHREAD_WIN32_)
unsigned WINAPI thread::wrapper_function(void * aArg)
#elif defined(_TTHREAD_POSIX_)
void * thread::wrapper_function(void * aArg)
#endif
{
  // Get thread wrapper information
  _thread_wrapper * tw = (_thread_wrapper *) aArg;

  try
  {
    // Call the actual client thread function
    tw->run();
  }
  catch(...)
  {
    // Uncaught exceptions will terminate the application (default behavior
    // according to C++11)
    std::terminate();
  }

  // The thread is no longer executing
  if(tw->release())
  {
    delete tw;
  }

  return 0;
}

thread::thread(void (*aFunction)(void *), void * aArg)
{
  // Fill out the thread startup information (passed to the thread wrapper)
  _thread_wrapper * tw = new _thread_wrapper(aFunction, aArg);

  // Create the thread
#if defined(_TTHREAD_WIN32_)
  mHandle = (HANDLE) _beginthreadex(0, 0, wrapper_function, (void *) tw, 0, &mWin32ThreadID);
#elif defined(_TTHREAD_POSIX_)
  if(pthread_create(&mHandle, NULL, wrapper_function, (void *) tw) != 0)
    mHandle = 0;
#endif

  // Did we fail to create the thread?
  if(!mHandle)
  {
    delete tw;
    tw = 0;
  }

  mWrapper = (void *) tw;
}

thread::~thread()
{
  _thread_wrapper * tw = static_cast<_thread_wrapper*>(mWrapper);
  if(!tw)
    return;

  if(tw->release())
  {
    delete tw;
  }
  else
  {
    // If the thread wrapper was not released, the thread is still joinable,
    // which should result in std::terminate() upon destruction according to
    // spec.
    std::terminate();
  }
}

void thread::join()
{
  _thread_wrapper * tw = static_cast<_thread_wrapper*>(mWrapper);
  if(!tw)
    return;

  if(tw->joinable())
  {
#if defined(_TTHREAD_WIN32_)
    WaitForSingleObject(mHandle, INFINITE);
    CloseHandle(mHandle);
#elif defined(_TTHREAD_POSIX_)
    pthread_join(mHandle, NULL);
#endif
  }

  // Note: At this point release() should always return true, since the
  // wrapper object should already have been released in the thread before
  // joining.
  if(tw->release())
  {
    delete tw;
  }
  mWrapper = 0;
}

bool thread::joinable() const
{
  _thread_wrapper * tw = static_cast<_thread_wrapper*>(mWrapper);
  if(!tw)
    return false;

  return tw->joinable();
}

void thread::detach()
{
  _thread_wrapper * tw = static_cast<_thread_wrapper*>(mWrapper);
  if(!tw)
    return;

#if defined(_TTHREAD_WIN32_)
  CloseHandle(mHandle);
#elif defined(_TTHREAD_POSIX_)
  pthread_detach(mHandle);
#endif

  if(tw->release())
  {
    delete tw;
  }
  mWrapper = 0;
}

thread::id thread::get_id() const
{
  if(!joinable())
    return id();
#if defined(_TTHREAD_WIN32_)
  return id((unsigned long int) mWin32ThreadID);
#elif defined(_TTHREAD_POSIX_)
  return _pthread_t_to_ID(mHandle);
#endif
}

unsigned thread::hardware_concurrency()
{
#if defined(_TTHREAD_WIN32_)
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return (int) si.dwNumberOfProcessors;
#elif defined(_SC_NPROCESSORS_ONLN)
  return (int) sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(_SC_NPROC_ONLN)
  return (int) sysconf(_SC_NPROC_ONLN);
#else
  // The standard requires this function to return zero if the number of
  // hardware cores could not be determined.
  return 0;
#endif
}


//------------------------------------------------------------------------------
// this_thread
//------------------------------------------------------------------------------

thread::id this_thread::get_id()
{
#if defined(_TTHREAD_WIN32_)
  return thread::id((unsigned long int) GetCurrentThreadId());
#elif defined(_TTHREAD_POSIX_)
  return _pthread_t_to_ID(pthread_self());
#endif
}

}
