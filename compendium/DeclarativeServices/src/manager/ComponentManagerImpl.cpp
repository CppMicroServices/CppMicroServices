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

            GetState()->Disable(*this, nullptr);
            for (auto& fut : disableFutures)
            {
                try
                {
                    fut.get();
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
        ComponentManagerImpl::Initialize()
        {
            if (compDesc->enabled)
            {
                std::atomic<bool>* nonce = new std::atomic<bool>(false);
                auto fut = Enable(nonce);
                try
                {
                    auto f = fut.wait_for(std::chrono::milliseconds(50)) == std::future_status::timeout;
                    US_UNUSED(f);
                    while (fut.wait_for(std::chrono::milliseconds(50)) == std::future_status::timeout)
                    {
                        auto expected = false;
                        auto desired = true;
                        if (std::atomic_compare_exchange_strong(nonce, &expected, desired))
                        {
                            auto task = taskMap[nonce];
                            std::shared_ptr<CMEnabledState> enabledState
                                = std::make_shared<CMEnabledState>((*task).get_future().share());
                            (*task)(enabledState);
                        }
                    }
                    delete nonce;
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
        ComponentManagerImpl::Enable(std::atomic<bool>* nonce)
        {
            return GetState()->Enable(*this, nonce);
        }

        std::shared_future<void>
        ComponentManagerImpl::Disable(std::atomic<bool>* nonce)
        {
            return GetState()->Disable(*this, nonce);
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

        void
        ComponentManagerImpl::AccumulateFuture(std::shared_future<void> fObj)
        {
            std::lock_guard<std::mutex> lk(futuresMutex);
            auto iterator
                = std::find_if(disableFutures.begin(), disableFutures.end(), is_ready<std::shared_future<void>&>);
            if (iterator == disableFutures.end())
            {
                disableFutures.push_back(fObj);
            }
            else // swap the ready future with the new one
            {
                std::swap(*iterator, fObj);
            }
        }

        std::shared_future<void>
        ComponentManagerImpl::PostAsyncDisabledToEnabled(
            std::shared_ptr<cppmicroservices::scrimpl::ComponentManagerState>& currentState,
            std::atomic<bool>* nonce)
        {
            auto metadata = GetMetadata();
            auto bundle = GetBundle();
            auto reg = GetRegistry();
            auto logger = GetLogger();
            auto configNotifier = GetConfigNotifier();

            using ActualTask = std::packaged_task<void(std::shared_ptr<CMEnabledState>)>;
            using PostTask = std::packaged_task<void()>;

            ActualTask task(
                [metadata, bundle, reg, logger, configNotifier, nonce](std::shared_ptr<CMEnabledState> eState) mutable
                {
                    bool expected = false;
                    bool desired = true;
                    // if nonce is non null this is a blocking call
                    // and the value is true, this task has already executed
                    if (nonce && !std::atomic_compare_exchange_strong(nonce, &expected, desired))
                    {
                        return;
                    }
                    eState->CreateConfigurations(metadata, bundle, reg, logger, configNotifier);
                });

            std::shared_ptr<CMEnabledState> enabledState = std::make_shared<CMEnabledState>(task.get_future().share());

            auto taskPtr_ = std::make_shared<ActualTask>(std::move(task));
            taskMap[nonce] = taskPtr_;

            PostTask post_task([enabledState, taskPtr = taskPtr_]() mutable { (*taskPtr)(enabledState); });

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
            std::atomic<bool>* nonce)
        {
            using ActualTask = std::packaged_task<void(std::shared_ptr<CMEnabledState>)>;
            using PostTask = std::packaged_task<void()>;

            ActualTask task(
                [nonce](std::shared_ptr<CMEnabledState> enabledState) mutable
                {
                    bool expected = false;
                    bool desired = true;
                    // if nonce is non null this is a blocking call
                    // and the value is true, this task has already executed
                    if (nonce && !std::atomic_compare_exchange_strong(nonce, &expected, desired))
                    {
                        return;
                    }
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

                auto taskPtr_ = std::make_shared<ActualTask>(std::move(task));
                taskMap[nonce] = taskPtr_;

                PostTask post_task([currEnabledState, taskPtr = taskPtr_]() mutable { (*taskPtr)(currEnabledState); });

                asyncWorkService->post(std::move(post_task));

                auto fut = disabledState->GetFuture();
                AccumulateFuture(fut);
                return fut;
            }
            // return the stored future in the current disabled state object
            return currentState->GetFuture();
        }
    } // namespace scrimpl
} // namespace cppmicroservices
