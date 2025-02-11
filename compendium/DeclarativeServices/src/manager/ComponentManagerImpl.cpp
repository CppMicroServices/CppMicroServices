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
#include "SingleInvokeTask.hpp"
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
            auto singleInvoke = std::make_shared<SingleInvokeTask>();
            GetState()->Disable(*this, singleInvoke);
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
                                            std::shared_ptr<SingleInvokeTask> singleInvoke)
        {
            constexpr auto timeout = std::chrono::milliseconds(50);
            // if we hit the timeout
            if (fut.wait_for(timeout) == std::future_status::timeout)
            {
                (*singleInvoke)();
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
                auto singleInvoke = std::make_shared<SingleInvokeTask>();
                auto fut = Enable(singleInvoke);
                try
                {
                    WaitForFuture(fut, singleInvoke);
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
        ComponentManagerImpl::Enable(std::shared_ptr<SingleInvokeTask> singleInvoke)
        {
            if (!singleInvoke)
            {
                singleInvoke = std::make_shared<SingleInvokeTask>();
            }
            return GetState()->Enable(*this, singleInvoke);
        }

        std::shared_future<void>
        ComponentManagerImpl::Disable(std::shared_ptr<SingleInvokeTask> singleInvoke)
        {
            if (!singleInvoke)
            {
                singleInvoke = std::make_shared<SingleInvokeTask>();
            }
            return GetState()->Disable(*this, singleInvoke);
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
        isReady(std::pair<std::shared_future<void>, std::shared_ptr<SingleInvokeTask>> obj)
        {
            return (is_ready(obj.first));
        }
        void
        ComponentManagerImpl::AccumulateFuture(std::shared_future<void> fObj,
                                               std::shared_ptr<SingleInvokeTask> singleInvoke)
        {
            std::pair<std::shared_future<void>, std::shared_ptr<SingleInvokeTask>> pair
                = std::make_pair(fObj, singleInvoke);
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
            std::shared_ptr<SingleInvokeTask> singleInvoke)
        {
            auto metadata = GetMetadata();
            auto bundle = GetBundle();
            auto reg = GetRegistry();
            auto logger = GetLogger();
            auto configNotifier = GetConfigNotifier();

            using ActualTask = std::packaged_task<void(std::shared_ptr<CMEnabledState>)>;
            using PostTask = std::packaged_task<void()>;

            ActualTask task(
                [metadata, bundle, reg, logger, configNotifier](std::shared_ptr<CMEnabledState> eState) mutable
                {
                    // do the task
                    eState->CreateConfigurations(metadata, bundle, reg, logger, configNotifier);
                });

            std::shared_ptr<CMEnabledState> enabledState = std::make_shared<CMEnabledState>(task.get_future().share());

            auto taskPtr = std::make_shared<ActualTask>(std::move(task));

            PostTask inter_task([enabledState, taskPtr]() mutable { (*taskPtr)(enabledState); });
            auto interTaskPtr = std::make_shared<PostTask>(std::move(inter_task));

            singleInvoke->addTask(interTaskPtr);

            PostTask post_task([singleInvoke]() mutable { (*singleInvoke)(); });

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
            std::shared_ptr<SingleInvokeTask> singleInvoke)
        {
            using ActualTask = std::packaged_task<void(std::shared_ptr<CMEnabledState>)>;
            using PostTask = std::packaged_task<void()>;

            ActualTask task(
                [](std::shared_ptr<CMEnabledState> enabledState) mutable
                {
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

                PostTask interTask([currEnabledState, taskPtr]() mutable { (*taskPtr)(currEnabledState); });

                auto interTaskPtr = std::make_shared<PostTask>(std::move(interTask));
                singleInvoke->addTask(interTaskPtr);
                PostTask post_task([singleInvoke]() mutable { (*singleInvoke)(); });

                asyncWorkService->post(std::move(post_task));

                auto fut = disabledState->GetFuture();
                AccumulateFuture(fut, singleInvoke);
                return fut;
            }
            // return the stored future in the current disabled state object
            return currentState->GetFuture();
        }
    } // namespace scrimpl
} // namespace cppmicroservices
