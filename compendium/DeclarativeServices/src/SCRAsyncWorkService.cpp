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
 * that a AsyncWorkService is not present within the framework. It implements
 * the public interface for AsyncWorkService and is created in the event that
 * a user-provided service was not given or if the user-provided service
 * which implements the AsyncWorkService interface was unregistered.
 */
class SCRAsyncWorkServiceDetail
  : public cppmicroservices::async::detail::AsyncWorkService
{
public:
  SCRAsyncWorkServiceDetail() { Initialize(); }

  void Initialize()
  {
    threadpool = std::make_shared<boost::asio::thread_pool>(2);
  }

  void Shutdown()
  {
    try {
      if (threadpool) {
        try {
          threadpool->join();
          threadpool->stop();
          threadpool.reset();
        } catch (...) {
          //
        }
      }
    } catch (...) {
      //
    }
  }

  ~SCRAsyncWorkServiceDetail() { Shutdown(); }

  void post(std::packaged_task<void()>&& task) override
  {
    if (threadpool) {
      using Sig = void();
      using Result = boost::asio::async_result<decltype(task), Sig>;
      using Handler = typename Result::completion_handler_type;

      Handler handler(std::forward<decltype(task)>(task));
      Result result(handler);

      boost::asio::post(
        threadpool->get_executor(),
        [handler = std::move(handler)]() mutable { handler(); });
    }
  }

private:
  std::shared_ptr<boost::asio::thread_pool> threadpool;
};

SCRAsyncWorkService::SCRAsyncWorkService(
  cppmicroservices::BundleContext context,
  std::shared_ptr<SCRLogger>& logger_)
  : scrContext(context)
  , serviceTracker(
      std::make_unique<cppmicroservices::ServiceTracker<
        cppmicroservices::async::detail::AsyncWorkService>>(context, this))
  , asyncWorkService(std::make_shared<SCRAsyncWorkServiceDetail>())
  , logger(logger_)
{
  serviceTracker->Open();
}

SCRAsyncWorkService::~SCRAsyncWorkService()
{
  asyncWorkService.reset();
}

void SCRAsyncWorkService::StopTracking()
{
  if (serviceTracker) {
    serviceTracker->Close();
    serviceTracker.reset();
  }
}

std::shared_ptr<cppmicroservices::async::detail::AsyncWorkService>
SCRAsyncWorkService::AddingService(
  const ServiceReference<cppmicroservices::async::detail::AsyncWorkService>&
    reference)
{
  auto currAsync = std::atomic_load(&asyncWorkService);
  std::shared_ptr<cppmicroservices::async::detail::AsyncWorkService> newService;
  if (reference) {
    try {
      newService =
        scrContext
          .GetService<cppmicroservices::async::detail::AsyncWorkService>(
            reference);
      if (newService) {
        std::atomic_store(&asyncWorkService, newService);
      }
    } catch (...) {
      std::string msg = "An exception occured while trying to get the "
                        "user-provided implement of AsyncWorkService. "
                        "Defaulting to the provided fallback "
                        "solution";
      logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING,
                  msg);
    }
  }
  return newService;
}

void SCRAsyncWorkService::ModifiedService(
  const ServiceReference<
    cppmicroservices::async::detail::AsyncWorkService>& /* reference */,
  const std::shared_ptr<
    cppmicroservices::async::detail::AsyncWorkService>& /* service */)
{
  // no-op
}

void SCRAsyncWorkService::RemovedService(
  const ServiceReference<
    cppmicroservices::async::detail::AsyncWorkService>& /* reference */,
  const std::shared_ptr<cppmicroservices::async::detail::AsyncWorkService>&
    service)
{
  auto currAsync = std::atomic_load(&asyncWorkService);
  if (service == currAsync) {
    // replace existing asyncWorkService with a nullptr asyncWorkService
    std::shared_ptr<cppmicroservices::async::detail::AsyncWorkService>
      newService = std::make_shared<SCRAsyncWorkServiceDetail>();
    std::atomic_store(&asyncWorkService, newService);
  }
}

void SCRAsyncWorkService::post(std::packaged_task<void()>&& task)
{
  auto currAsync = std::atomic_load(&asyncWorkService);
  currAsync->post(std::move(task));
}

}
}
