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

#include "SCRAsyncWorkService.hpp"

#include "boost/asio/async_result.hpp"
#include "boost/asio/packaged_task.hpp"
#include "boost/asio/post.hpp"
#include "boost/asio/thread_pool.hpp"

namespace cppmicroservices {
namespace scrimpl {

/**
 * SCRAsyncWorkServiceDetail represents the fallback strategy in the event
 * that a AsyncWorkService is not present within the framework. It starts
 * in an "enabled" state when the SCRAsyncWorkService is initially created
 * and enters a disabled state once a valid service has been found (allowing 
 * threaded operations to be handled by the registered service). If that service
 * is then removed, the SCRAsyncWorkServiceDetail class becoems "enabled" and
 * threading operations will use the fallback strategy.
 */
class SCRAsyncWorkServiceDetail
{
public:
  SCRAsyncWorkServiceDetail() { Enable(); }

  void Enable()
  {
    if (!threadpool) {
      threadpool = std::make_shared<boost::asio::thread_pool>(2);
    }
  }

  void Disable()
  {
    try {
      if (threadpool) {
        try {
          threadpool->join();
          threadpool.reset();
          threadpool = nullptr;
        } catch (...) {
          //
        }
      }
    } catch (...) {
      //
    }
  }

  ~SCRAsyncWorkServiceDetail() { Disable(); }

  void post(std::packaged_task<void()>&& task)
  {
    using Sig = void();
    using Result = boost::asio::async_result<decltype(task), Sig>;
    using Handler = typename Result::completion_handler_type;

    Handler handler(std::forward<decltype(task)>(task));
    Result result(handler);

    boost::asio::post(threadpool->get_executor(),
                      [handler = std::move(handler)]() mutable { handler(); });
  }

private:
  std::shared_ptr<boost::asio::thread_pool> threadpool;
};

SCRAsyncWorkService::SCRAsyncWorkService(
  cppmicroservices::BundleContext context)
  : scrContext(context)
  , serviceTracker(
      std::make_unique<
        cppmicroservices::ServiceTracker<cppmsasync::AsyncWorkService>>(
        context))
  , asyncWorkService(nullptr)
  , detail(std::make_unique<SCRAsyncWorkServiceDetail>())
{
  serviceTracker->Open();
}

SCRAsyncWorkService::~SCRAsyncWorkService()
{
  asyncWorkService.reset();
  if (detail) {
    detail->Disable();
  }
}

void SCRAsyncWorkService::StopTracking()
{
  if (serviceTracker) {
    serviceTracker->Close();
    serviceTracker.reset();
  }
}

std::shared_ptr<cppmsasync::AsyncWorkService>
SCRAsyncWorkService::AddingService(
  const ServiceReference<cppmsasync::AsyncWorkService>& reference)
{
  auto currAsync = std::atomic_load(&asyncWorkService);
  std::shared_ptr<cppmsasync::AsyncWorkService> newService;
  if (!currAsync && reference) {
    try {
      newService =
        scrContext.GetService<cppmsasync::AsyncWorkService>(reference);
      if (newService) {
        std::atomic_store(&asyncWorkService, newService);
        detail->Disable();
      }
    } catch (...) {
      newService = nullptr;
      std::atomic_store(&asyncWorkService, newService);
    }
  }
  return newService;
}

void SCRAsyncWorkService::ModifiedService(
  const ServiceReference<cppmsasync::AsyncWorkService>& /* reference */,
  const std::shared_ptr<cppmsasync::AsyncWorkService>& /* service */)
{
  // no-op
}

void SCRAsyncWorkService::RemovedService(
  const ServiceReference<cppmsasync::AsyncWorkService>& /* reference */,
  const std::shared_ptr<cppmsasync::AsyncWorkService>& service)
{
  auto currAsync = std::atomic_load(&asyncWorkService);
  if (service == currAsync) {
    // replace existing asyncWorkService with a nullptr asyncWorkService
    std::shared_ptr<cppmsasync::AsyncWorkService> newService(nullptr);
    std::atomic_store(&asyncWorkService, newService);
    detail->Enable();
  }
}

void SCRAsyncWorkService::post(std::packaged_task<void()>&& task)
{
  auto currAsync = std::atomic_load(&asyncWorkService);
  if (currAsync) {
    currAsync->post(std::move(task));
  } else {
    // The default threading solution should be enabled here.
    detail->post(std::move(task));
  }
}

}
}
