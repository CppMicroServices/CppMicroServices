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

#ifndef USBUNDLETHREAD_P_H
#define USBUNDLETHREAD_P_H

#include <thread>
#include <atomic>
#include <chrono>
#include <string>

#include "usBundleEvent.h"

#include "usThreads_p.h"
#include "usWaitCondition_p.h"

#include <future>

namespace us {

class BundlePrivate;
class CoreBundleContext;

class BundleThread : public MultiThreaded<MutexLockingStrategy<>, WaitCondition>
    , public std::enable_shared_from_this<BundleThread>
{
  const static int OP_IDLE;
  const static int OP_BUNDLE_EVENT;
  const static int OP_START;
  const static int OP_STOP;

  const static std::chrono::milliseconds KEEP_ALIVE;

  CoreBundleContext* const fwCtx;
  std::chrono::milliseconds startStopTimeout;

  BundleEvent be;
  std::atomic<BundlePrivate*> bundle;
  std::atomic<int> operation;
  std::promise<bool> pr;
  std::atomic<bool> doRun;

  struct : MultiThreaded<> { std::thread v; } th;

public:

  BundleThread(CoreBundleContext* ctx);

  void Quit();

  void Run();

  void Join();

  /**
   * Note! Must be called while holding packages lock.
   */
  void BundleChanged(const BundleEvent& be, UniqueLock& resolveLock);

  /**
   * Note! Must be called while holding packages lock.
   */
  std::exception_ptr CallStart0(BundlePrivate* b, UniqueLock& resolveLock);

  /**
   * Note! Must be called while holding packages lock.
   */
  std::exception_ptr CallStop1(BundlePrivate* b, UniqueLock& resolveLock);

  /**
   * Note! Must be called while holding packages lock.
   */
  std::exception_ptr StartAndWait(BundlePrivate* b, int op, UniqueLock& resolveLock);

  bool IsExecutingBundleChanged() const;

  bool operator==(const std::thread::id& id) const;

};

}

#endif // USBUNDLETHREAD_P_H
