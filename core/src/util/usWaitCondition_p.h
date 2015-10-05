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

#ifndef USWAITCONDITION_P_H
#define USWAITCONDITION_P_H

#include "usGlobalConfig.h"

#include <chrono>
#include <condition_variable>

US_BEGIN_NAMESPACE

template<class MutexHost>
class WaitCondition
{
public:

  template<class Rep, class Period>
  std::cv_status WaitFor(typename MutexHost::Lock& lock, const std::chrono::duration<Rep, Period>& rel_time)
  {
#ifdef US_ENABLE_THREADING_SUPPORT
    return m_CondVar.wait_for(lock.m_Lock, rel_time);
#else
    US_UNUSED(lock);
    US_UNUSED(rel_time);
    return std::cv_status::no_timeout;
#endif
  }

  /** Notify that the condition is true and release one waiting thread */
  void Notify()
  {
#ifdef US_ENABLE_THREADING_SUPPORT
    m_CondVar.notify_one();
#endif
  }

  /** Notify that the condition is true and release all waiting threads */
  void NotifyAll()
  {
#ifdef US_ENABLE_THREADING_SUPPORT
    m_CondVar.notify_all();
#endif
  }

private:

#ifdef US_ENABLE_THREADING_SUPPORT
  std::condition_variable m_CondVar;
#endif
};

US_END_NAMESPACE

#endif // USWAITCONDITION_P_H
