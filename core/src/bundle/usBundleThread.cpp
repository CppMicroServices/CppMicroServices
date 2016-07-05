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

#include "usBundleThread_p.h"

#include "usBundleActivator.h"
#include "usBundlePrivate.h"
#include "usCoreBundleContext_p.h"

#include <future>

namespace us {

const int BundleThread::OP_IDLE = 0;
const int BundleThread::OP_BUNDLE_EVENT = 1;
const int BundleThread::OP_START = 2;
const int BundleThread::OP_STOP = 3;

const std::chrono::milliseconds BundleThread::KEEP_ALIVE(1000);

BundleThread::BundleThread(CoreBundleContext* ctx)
  : fwCtx(ctx)
  , startStopTimeout(0)
  , bundle(nullptr)
  , operation(OP_IDLE)
  , doRun(true)
{
  th.v = std::thread(&BundleThread::Run, this);
}

void BundleThread::Quit()
{
  doRun = false;
  NotifyAll();
  auto l = th.Lock(); US_UNUSED(l);
  if (th.v.joinable()) th.v.join();
}

void BundleThread::Run()
{
  while (doRun)
  {
    {
      auto l = Lock();
      while (doRun && operation == OP_IDLE)
      {
        WaitFor(l, KEEP_ALIVE);
        if (!doRun) return;
        if (operation != OP_IDLE)
        {
          break;
        }
        {
          auto l2 = fwCtx->bundleThreads.Lock(); US_UNUSED(l2);
          auto iter = std::find(fwCtx->bundleThreads.value.begin(), fwCtx->bundleThreads.value.end(), this->shared_from_this());
          if (iter != fwCtx->bundleThreads.value.end())
          {
            fwCtx->bundleThreads.zombies.push_back(*iter);
            fwCtx->bundleThreads.value.erase(iter);
            return;
          }
        }
      }

      if (!doRun) return;

      std::exception_ptr tmpres;
      try
      {
        switch (operation.load())
        {
        case OP_BUNDLE_EVENT:
          fwCtx->listeners.BundleChanged(be);
          break;
        case OP_START:
          tmpres = bundle.load()->Start0();
          break;
        case OP_STOP:
          tmpres = bundle.load()->Stop1();
          break;
        }
      }
      catch (const std::exception& e)
      {
        US_ERROR << bundle.load()->symbolicName << ": " << e.what();
      }

      operation = OP_IDLE;
      if (tmpres)
      {
        pr.set_exception(tmpres);
      }
      else
      {
        pr.set_value(true);
      }

    }

    // lock the resolver in order to synchronize with StartAndWait
    fwCtx->resolver.Lock(), fwCtx->resolver.NotifyAll();
  }
}

void BundleThread::Join()
{
  auto l = th.Lock(); US_UNUSED(l);
  if (th.v.joinable())
  {
    th.v.join();
  }
}

void BundleThread::BundleChanged(const BundleEvent& be, UniqueLock& resolveLock)
{
  Lock(), this->be = be;
  StartAndWait(GetPrivate(be.GetBundle()).get(), OP_BUNDLE_EVENT, resolveLock);
}

std::exception_ptr BundleThread::CallStart0(BundlePrivate* b, UniqueLock& resolveLock)
{
  return StartAndWait(b, OP_START, resolveLock);
}

std::exception_ptr BundleThread::CallStop1(BundlePrivate* b, UniqueLock& resolveLock)
{
  return StartAndWait(b, OP_STOP, resolveLock);
}

std::exception_ptr BundleThread::StartAndWait(BundlePrivate* b, int op, UniqueLock& resolveLock)
{
  std::future<bool> res;
  {
    auto l = Lock(); US_UNUSED(l);
    pr = std::promise<bool>();
    res = pr.get_future();
    bundle = b;
    operation = op;
  }
  NotifyAll();

  // timeout for waiting on op to finish can be set for start/stop
  auto waitTime = std::chrono::milliseconds::zero();
  if (op == OP_START || op == OP_STOP)
  {
    b->aborted = static_cast<uint8_t>(BundlePrivate::Aborted::NO); // clear aborted status
    waitTime = startStopTimeout;
  }
  bool timeout = false;
  bool uninstall = false;

  fwCtx->resolver.WaitFor(resolveLock, waitTime, [&res]{
    return res.valid() &&
        res.wait_for(std::chrono::milliseconds::zero()) == US_FUTURE_READY;
  });

  // Abort start/stop operation if bundle has been uninstalled
  if ((op == OP_START || op == OP_STOP) && b->state == Bundle::STATE_UNINSTALLED)
  {
    uninstall = true;
  }
  else if (waitTime.count() > 0 && // we were waiting with a timeout
           ((op == OP_START && b->state == Bundle::STATE_STARTING)
            || (op == OP_STOP && b->state == Bundle::STATE_STOPPING))
           )
  {
    timeout = true;
  }

  // if b->aborted is set, BundleThread has/will concluded start/stop
  if (b->aborted == static_cast<uint8_t>(BundlePrivate::Aborted::NONE) &&
      (timeout || uninstall))
  {
    // BundleThread is still in BundleActivator::Start/::Stop,

    // signal to BundleThread that this
    // thread is acting on uninstall/time-out
    b->aborted = static_cast<uint8_t>(BundlePrivate::Aborted::YES);

    std::string opType = op == OP_START ? "start" : "stop";
    std::string reason = timeout ? "Time-out during bundle " + opType + "()"
                                 : "Bundle uninstalled during " + opType + "()";

    US_INFO << "bundle thread aborted during " << opType
            << " of bundle #" << b->id;

    if (timeout)
    {
      if (op == OP_START)
      {
        // set state, send events, do clean-up like when Bundle::Start()
        // throws an exception
        // TODO: StartFailed() calls BundleListener::BundleChanged and
        // should not be called with the packages lock as we do here
        b->StartFailed();
      }
      else
      {
        // STOP, like when Bundle::Stop() returns/throws an exception
        b->bactivator.reset();
        b->Stop2();
      }
    }

    Quit();
    b->ResetBundleThread();

    return std::make_exception_ptr(std::runtime_error(
                                     "Bundle#" + std::to_string(b->id) + " " +
                                     opType + " failed with reason: " + reason
                                     ));
  }
  else
  {
    {
      fwCtx->bundleThreads.Lock(), fwCtx->bundleThreads.value.push_front(this->shared_from_this());
      if (op != operation)
      {
        // TODO! Handle when operation has changed.
        // i.e. uninstall during operation?
      }
      b->ResetBundleThread();
      try
      {
        res.get();
        return nullptr;
      }
      catch (...)
      {
        return std::current_exception();
      }
    }
  }
}

bool BundleThread::IsExecutingBundleChanged() const
{
  return operation == OP_BUNDLE_EVENT;
}

bool BundleThread::operator==(const std::thread::id& id) const
{
  return (th.Lock(), th.v.get_id() == id);
}

}
