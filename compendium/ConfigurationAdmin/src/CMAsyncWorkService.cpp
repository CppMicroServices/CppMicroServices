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

#include "CMAsyncWorkService.hpp"

#include "boost/asio/async_result.hpp"
#include "boost/asio/packaged_task.hpp"
#include "boost/asio/post.hpp"
#include "boost/asio/thread_pool.hpp"
#include <boost/asio.hpp>

namespace cppmicroservices
{
    namespace cmimpl
    {
        using AWSInt = cppmicroservices::async::AsyncWorkService;
        /**
         * FallbackAsyncWorkService represents the fallback strategy in the event
         * that a AsyncWorkService is not present within the framework. It implements
         * the public interface for AsyncWorkService and is created in the event that
         * a user-provided service was not given or if the user-provided service
         * which implements the AsyncWorkService interface was unregistered.
         */
        class FallbackAsyncWorkService final : public AWSInt
        {
          public:
            using Strand = boost::asio::strand<boost::asio::thread_pool::executor_type>;

            FallbackAsyncWorkService(std::shared_ptr<cppmicroservices::logservice::LogService> const& logger_)
                : logger(logger_)
            {
                Initialize();
            }

            void
            Initialize()
            {
                threadpool = std::make_shared<boost::asio::thread_pool>(1);
            }

            void
            Shutdown()
            {
                if (threadpool)
                {
                    try
                    {
                        threadpool->join();
                        threadpool->stop();
                        threadpool.reset();
                    }
                    catch (...)
                    {
                        auto exceptionPtr = std::current_exception();
                        std::string msg = "An exception has occurred while trying to shutdown "
                                          "the fallback cppmicroservices::async::AsyncWorkService "
                                          "instance.";
                        logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING, msg, exceptionPtr);
                    }
                }
            }

            ~FallbackAsyncWorkService() { Shutdown(); }

            void
            post(std::packaged_task<void()>&& task) override
            {
                if (threadpool)
                {
                    boost::asio::post(threadpool->get_executor(), std::move(task));
                }
            }

            // createStrand returns a new AsyncWorkService instance posting to a strand
            std::shared_ptr<AsyncWorkService>
            createStrand() override
            {
                if (!threadpool)
                {
                    return nullptr;
                }
                auto strand = std::make_shared<Strand>(threadpool->get_executor());
                // Pass shared_ptr to threadpool to keep it alive as long as strand is alive
                return std::make_shared<StrandAsyncWorkService>(strand, logger);
            }

          private:
            std::shared_ptr<boost::asio::thread_pool> threadpool;
            std::shared_ptr<cppmicroservices::logservice::LogService> logger;

            // Inner class for strand-based async work service
            class StrandAsyncWorkService : public AsyncWorkService
            {
              public:
                StrandAsyncWorkService(std::shared_ptr<Strand> strand_,
                                       std::shared_ptr<cppmicroservices::logservice::LogService> logger_)
                    : strand(std::move(strand_))
                    , logger(std::move(logger_))
                {
                }

                void
                post(std::packaged_task<void()>&& task) override
                {
                    if (strand)
                    {
                        boost::asio::post(*strand, std::move(task));
                    }
                }

              private:
                std::shared_ptr<Strand> strand;
                std::shared_ptr<cppmicroservices::logservice::LogService> logger;
            };
        };

        CMAsyncWorkService::CMAsyncWorkService(cppmicroservices::BundleContext context,
                                               std::shared_ptr<cppmicroservices::logservice::LogService> const& logger_)
            : scrContext(context)
            , serviceTracker(std::make_unique<cppmicroservices::ServiceTracker<AWSInt>>(context, this))
            , usingFallback(true)
            , asyncWorkService(nullptr)
            , logger(logger_)
        {
            {
                if (auto asyncWSSRef = context.GetServiceReference<AWSInt>(); asyncWSSRef)
                {
                    usingFallback = false;
                    currRef = asyncWSSRef;
                    asyncWorkService = context.GetService<AWSInt>(asyncWSSRef);
                }
                else
                {
                    usingFallback = true;
                    asyncWorkService = std::make_shared<FallbackAsyncWorkService>(logger_);
                }
            }
            serviceTracker->Open();
        }

        CMAsyncWorkService::~CMAsyncWorkService() noexcept { asyncWorkService.reset(); }

        void
        CMAsyncWorkService::StopTracking()
        {
            if (serviceTracker)
            {
                serviceTracker->Close();
                serviceTracker.reset();
            }
        }

        std::shared_ptr<AWSInt>
        CMAsyncWorkService::AddingService(ServiceReference<AWSInt> const& reference)
        {
            std::unique_lock<std::mutex> lock { m };
            auto currAsync = asyncWorkService;
            std::shared_ptr<AWSInt> newService;
            if (reference)
            {
                try
                {
                    newService = scrContext.GetService<AWSInt>(reference);
                    // if the new ref exists and:
                    // we are using the fallback OR
                    // our current < new (based on ranking and id), reassign
                    if (newService && (usingFallback || currRef < reference))
                    {
                        asyncWorkService = newService;
                    }
                }
                catch (...)
                {
                    auto exceptionPtr = std::current_exception();
                    std::string msg = "An exception was caught while retrieving an instance of "
                                      "cppmicroservices::async::AsyncWorkService. Falling "
                                      "back to the default.";
                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING, msg, exceptionPtr);
                }
            }
            return newService;
        }

        void
        CMAsyncWorkService::ModifiedService(ServiceReference<AWSInt> const& /* reference */,
                                            std::shared_ptr<AWSInt> const& /* service */)
        {
            // no-op
        }

        void
        CMAsyncWorkService::RemovedService(ServiceReference<AWSInt> const& /* reference */,
                                           std::shared_ptr<AWSInt> const& service)
        {
            std::unique_lock<std::mutex> lock { m };
            auto currAsync = asyncWorkService;
            if (service == currAsync)
            {
                currRef = ServiceReference<AWSInt>();
                usingFallback = true;
                // replace existing asyncWorkService with a fallbackAsyncWorkService
                asyncWorkService = std::make_shared<FallbackAsyncWorkService>(logger);
            }
        }

        void
        CMAsyncWorkService::post(std::packaged_task<void()>&& task)
        {
            std::unique_lock<std::mutex> lock { m };
            asyncWorkService->post(std::move(task));
        }

        std::shared_ptr<AWSInt>
        CMAsyncWorkService::createStrand()
        {
            return asyncWorkService->createStrand();
        }

    } // namespace cmimpl
} // namespace cppmicroservices
