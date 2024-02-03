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

#include "ComponentManagerImpl.hpp"
#include "ConcurrencyUtil.hpp"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/SharedLibraryException.h"
#include "cppmicroservices/detail/ScopeGuard.h"
#include "states/CMDisabledState.hpp"
#include "states/ComponentManagerState.hpp"
#include <cassert>
#include <future>
#include <utility>

namespace cppmicroservices
{
    namespace scrimpl
    {
        ComponentManagerImpl::ComponentManagerImpl(
            std::shared_ptr<metadata::ComponentMetadata const> metadata,
            std::shared_ptr<ComponentRegistry> registry,
            BundleContext bundleContext,
            std::shared_ptr<cppmicroservices::logservice::LogService> logger,
            std::shared_ptr<cppmicroservices::async::AsyncWorkService> asyncWorkService,
            std::shared_ptr<ConfigurationNotifier> configNotifier)
            : registry(std::move(registry))
            , compDesc(std::move(metadata))
            , bundleContext(std::move(bundleContext))
            , logger(std::move(logger))
            , state(std::make_shared<CMDisabledState>())
            , asyncWorkService(std::move(asyncWorkService))
            , configNotifier(std::move(configNotifier))
        {
            if (!compDesc || !this->registry || !this->bundleContext || !this->logger || !this->asyncWorkService
                || !this->configNotifier)
            {
                throw std::invalid_argument("Invalid arguments to ComponentManagerImpl constructor");
            }
        }

        ComponentManagerImpl::~ComponentManagerImpl()
        {
            auto asyncStarted = std::make_shared<std::atomic<bool>>(false);
            GetState()->Disable(*this, asyncStarted);
            for (auto& pair : disableFutures)
            {
                try
                {
                    WaitForFuture(pair.first, pair.second);
                }
                catch (...)
                {
                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                "Exception while disabling component with name" + GetName(),
                                std::current_exception());
                }
            }
        }

        void
        ComponentManagerImpl::WaitForFuture(std::shared_future<void>& fut,
                                            std::shared_ptr<std::atomic<bool>> asyncStarted)
        {
            // ensure that asyncTaskMap and asyncTaskStateMap are cleared even if task throws
            detail::ScopeGuard sg(
                [this, asyncStarted]()
                {
                    asyncTaskMap.erase(asyncStarted);
                    asyncTaskStateMap.erase(asyncStarted);
                });

            constexpr auto timeout = std::chrono::milliseconds(50);
            // if we hit the timeout
            if (fut.wait_for(timeout) == std::future_status::timeout)
            {
                // we expect that the asyncStarted is false -- i.e. stalled
                auto expected = false;
                auto desired = true;
                // if it is *asyncStarted==false
                if (std::atomic_compare_exchange_strong(&(*asyncStarted), &expected, desired))
                {
                    // we execute the task
                    auto task = asyncTaskMap[asyncStarted];
                    auto enabledState = asyncTaskStateMap[asyncStarted];

                    // we pass in false because we always want to execute the task here
                    (*task)(enabledState, false);
                }
            }

            // we can always get the future... if stalled, it'll be satisfied by WFF
            // execution of the task else it will be satisfied by already executed object
            fut.get();
        }

        void
        ComponentManagerImpl::Initialize()
        {
            if (compDesc->enabled)
            {
                auto asyncStarted = std::make_shared<std::atomic<bool>>(false);
                auto fut = Enable(asyncStarted);
                try
                {
                    WaitForFuture(fut, asyncStarted);
                }
                catch (cppmicroservices::SharedLibraryException const&)
                {
                    throw;
                }
                catch (cppmicroservices::SecurityException const&)
                {
                    throw;
                }
                catch (...)
                {
                    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                "Failed to enable component with name" + GetName(),
                                std::current_exception());
                }
            }
        }

        bool
        ComponentManagerImpl::IsEnabled() const
        {
            return GetState()->IsEnabled(*this);
        }

        std::shared_future<void>
        ComponentManagerImpl::Enable(std::shared_ptr<std::atomic<bool>> asyncStarted)
        {
            return GetState()->Enable(*this, asyncStarted);
        }

        std::shared_future<void>
        ComponentManagerImpl::Disable(std::shared_ptr<std::atomic<bool>> asyncStarted)
        {
            return GetState()->Disable(*this, asyncStarted);
        }

        std::vector<std::shared_ptr<ComponentConfiguration>>
        ComponentManagerImpl::GetComponentConfigurations() const
        {
            return GetState()->GetConfigurations(*this);
        }

        std::shared_ptr<ComponentManagerState>
        ComponentManagerImpl::GetState() const
        {
            return std::atomic_load(&state);
        }

        bool
        ComponentManagerImpl::CompareAndSetState(std::shared_ptr<ComponentManagerState>* expectedState,
                                                 std::shared_ptr<ComponentManagerState> desiredState)
        {
            return std::atomic_compare_exchange_strong(&state, expectedState, desiredState);
        }

        bool
        isReady(std::pair<std::shared_future<void>, std::shared_ptr<std::atomic<bool>>> obj)
        {
            return (is_ready(obj.first));
        }
        void
        ComponentManagerImpl::AccumulateFuture(std::shared_future<void> fObj,
                                               std::shared_ptr<std::atomic<bool>> asyncStarted)
        {
            std::pair<std::shared_future<void>, std::shared_ptr<std::atomic<bool>>> pair
                = std::make_pair(fObj, asyncStarted);
            std::lock_guard<std::mutex> lk(futuresMutex);
            auto iterator = std::find_if(disableFutures.begin(), disableFutures.end(), isReady);
            if (iterator == disableFutures.end())
            {
                disableFutures.push_back(pair);
            }
            else // swap the ready future with the new one
            {
                std::swap(*iterator, pair);
            }
        }

        std::shared_future<void>
        ComponentManagerImpl::PostAsyncDisabledToEnabled(
            std::shared_ptr<cppmicroservices::scrimpl::ComponentManagerState>& currentState,
            std::shared_ptr<std::atomic<bool>> asyncStarted)
        {
            auto metadata = GetMetadata();
            auto bundle = GetBundle();
            auto reg = GetRegistry();
            auto logger = GetLogger();
            auto configNotifier = GetConfigNotifier();

            using ActualTask = std::packaged_task<void(std::shared_ptr<CMEnabledState>, bool)>;
            using PostTask = std::packaged_task<void()>;

            ActualTask task(
                [metadata, bundle, reg, logger, configNotifier, asyncStarted](std::shared_ptr<CMEnabledState> eState,
                                                                              bool checkExecuted) mutable
                {
                    // if this task is being run on spawned thread (not waiting thread), check execution status
                    if (checkExecuted)
                    {
                        bool expected = false;
                        bool desired = true;
                        // if asyncStarted is non null AND *asyncStarted==true
                        if (asyncStarted && !std::atomic_compare_exchange_strong(&(*asyncStarted), &expected, desired))
                        {
                            // this is blocking and it has started
                            return;
                        }
                        // else it is non-blocking or it has not started and we now own it
                    }
                    // do the task
                    eState->CreateConfigurations(metadata, bundle, reg, logger, configNotifier);
                });

            std::shared_ptr<CMEnabledState> enabledState = std::make_shared<CMEnabledState>(task.get_future().share());

            auto taskPtr = std::make_shared<ActualTask>(std::move(task));

            // if blocking, cache task and enabledState
            if (asyncStarted)
            {
                asyncTaskMap[asyncStarted] = taskPtr;
                asyncTaskStateMap[asyncStarted] = enabledState;
            }
            PostTask post_task(
                [enabledState, taskPtr]() mutable
                {
                    try
                    {
                        (*taskPtr)(enabledState, true);
                    }
                    catch (...)
                    {
                        /*
                         * task was already executed, by WaitForFuture, calling it
                         * again will throw. This is expected, we can just catch and continue
                         */
                    }
                });

            // if this object failed to change state and the current state is DISABLED, try again
            auto succeeded = false;
            std::lock_guard<std::mutex> lk(transitionMutex);
            do
            {
                succeeded = CompareAndSetState(&currentState, enabledState);
            } while (!succeeded && !currentState->IsEnabled(*this));

            if (succeeded) // succeeded in changing the state
            {
                asyncWorkService->post(std::move(post_task));
                return enabledState->GetFuture();
            }

            // return the stored future in the current enabled state object
            return currentState->GetFuture();
        }

        std::shared_future<void>
        ComponentManagerImpl::PostAsyncEnabledToDisabled(
            std::shared_ptr<cppmicroservices::scrimpl::ComponentManagerState>& currentState,
            std::shared_ptr<std::atomic<bool>> asyncStarted)
        {
            using ActualTask = std::packaged_task<void(std::shared_ptr<CMEnabledState>, bool)>;
            using PostTask = std::packaged_task<void()>;

            ActualTask task(
                [asyncStarted](std::shared_ptr<CMEnabledState> enabledState, bool checkExecuted) mutable
                {
                    if (checkExecuted)
                    {
                        bool expected = false;
                        bool desired = true;

                        // if asyncStarted is non null AND *asyncStarted==true
                        if (asyncStarted && !std::atomic_compare_exchange_strong(&(*asyncStarted), &expected, desired))
                        {
                            // this is blocking and it has started
                            return;
                        }
                        // else it is non-blocking or it has not started
                    }
                    // do task
                    enabledState->DeleteConfigurations();
                });

            auto disabledState = std::make_shared<CMDisabledState>(task.get_future().share());

            // if this object failed to change state and the current state is ENABLED, try again
            bool succeeded = false;
            std::lock_guard<std::mutex> lk(transitionMutex);
            do
            {
                succeeded = CompareAndSetState(&currentState, disabledState);
            } while (!succeeded && currentState->IsEnabled(*this));

            if (succeeded) // succeeded in changing the state
            {
                std::shared_ptr<CMEnabledState> currEnabledState
                    = std::dynamic_pointer_cast<CMEnabledState>(currentState);

                auto taskPtr = std::make_shared<ActualTask>(std::move(task));
                // if blocking, cache task and enabledState
                if (asyncStarted)
                {
                    asyncTaskMap[asyncStarted] = taskPtr;
                    asyncTaskStateMap[asyncStarted] = currEnabledState;
                }
                // this task goes to async queue, we wan't to check if it is already executed
                PostTask post_task(
                    [currEnabledState, taskPtr]() mutable
                    {
                        try
                        {
                            (*taskPtr)(currEnabledState, true);
                        }
                        catch (...)
                        {
                            /*
                             * task was already executed, by WaitForFuture, calling it
                             * again will throw. This is expected, we can just catch and continue
                             */
                        }
                    });

                asyncWorkService->post(std::move(post_task));

                auto fut = disabledState->GetFuture();
                AccumulateFuture(fut, asyncStarted);
                return fut;
            }
            // return the stored future in the current disabled state object
            return currentState->GetFuture();
        }
    } // namespace scrimpl
} // namespace cppmicroservices
