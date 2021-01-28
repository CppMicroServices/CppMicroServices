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

#include "boost/asio/async_result.hpp"
#include "boost/asio/packaged_task.hpp"
#include "boost/asio/post.hpp"
#include "ComponentManagerImpl.hpp"
#include "ConcurrencyUtil.hpp"
#include "cppmicroservices/SharedLibraryException.h"
#include "states/CMDisabledState.hpp"
#include "states/CMEnabledState.hpp"
#include "states/ComponentManagerState.hpp"
#include <cassert>
#include <future>
#include <utility>

namespace cppmicroservices {
namespace scrimpl {

ComponentManagerImpl::ComponentManagerImpl(std::shared_ptr<const metadata::ComponentMetadata> metadata,
                                           std::shared_ptr<const ComponentRegistry> registry,
                                           BundleContext bundleContext,
                                           std::shared_ptr<cppmicroservices::logservice::LogService> logger,
                                           std::shared_ptr<boost::asio::thread_pool> threadpool,
                                           std::shared_ptr<ConfigurationNotifier> configNotifier)
  : registry(std::move(registry))
  , compDesc(std::move(metadata))
  , bundleContext(std::move(bundleContext))
  , logger(std::move(logger))
  , state(std::make_shared<CMDisabledState>())
  , threadpool(threadpool)
  , configNotifier(configNotifier)
{
  if(!compDesc || !this->registry || !this->bundleContext || !this->logger || !this->threadpool || !this->configNotifier)
  {
    throw std::invalid_argument("Invalid arguments to ComponentManagerImpl constructor");
  }
}

ComponentManagerImpl::~ComponentManagerImpl()
{
  GetState()->Disable(*this);
  for(auto& fut : disableFutures)
  {
    try
    {
      fut.get();
    }
    catch(...)
    {
      logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR, "Exception while disabling component with name" + GetName(), std::current_exception());
    }
  }
}

void ComponentManagerImpl::Initialize()
{
  if(compDesc->enabled)
  {
    auto fut = Enable();
    try
    {
      fut.get();
    } catch (const cppmicroservices::SharedLibraryException&) {
      throw;
    } catch (...) {
      logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR, "Failed to enable component with name" + GetName(), std::current_exception());
    }
  }
}

bool ComponentManagerImpl::IsEnabled() const
{
  return GetState()->IsEnabled(*this);
}

std::shared_future<void> ComponentManagerImpl::Enable()
{
  return GetState()->Enable(*this);
}

std::shared_future<void> ComponentManagerImpl::Disable()
{
  return GetState()->Disable(*this);
}

std::vector<std::shared_ptr<ComponentConfiguration>> ComponentManagerImpl::GetComponentConfigurations() const
{
  return GetState()->GetConfigurations(*this);
}

std::shared_ptr<ComponentManagerState> ComponentManagerImpl::GetState() const
{
  return std::atomic_load(&state);
}

bool ComponentManagerImpl::CompareAndSetState(std::shared_ptr<ComponentManagerState>* expectedState, std::shared_ptr<ComponentManagerState> desiredState)
{
  return std::atomic_compare_exchange_strong(&state, expectedState, desiredState);
}

void ComponentManagerImpl::AccumulateFuture(std::shared_future<void> fObj)
{
  std::lock_guard<std::mutex> lk(futuresMutex);
  auto iterator = std::find_if(disableFutures.begin(), disableFutures.end(), is_ready<std::shared_future<void>&>);
  if(iterator == disableFutures.end())
  {
    disableFutures.push_back(fObj);
  }
  else // swap the ready future with the new one
  {
    std::swap(*iterator, fObj);
  }
}

std::shared_future<void> ComponentManagerImpl::PostAsyncDisabledToEnabled(
  std::shared_ptr<cppmicroservices::scrimpl::ComponentManagerState>&
    currentState)
{
  auto metadata = GetMetadata();
  auto bundle = GetBundle();
  auto reg = GetRegistry();
  auto logger = GetLogger();
  auto threadpool = GetThreadPool();
  auto configNotifier = GetConfigNotifier();
 
  std::packaged_task<void(std::shared_ptr<CMEnabledState>)> task(
    [metadata, bundle, reg, logger, threadpool, configNotifier](
      std::shared_ptr<CMEnabledState> eState) { eState->CreateConfigurations(
        metadata, bundle, reg, logger, threadpool, configNotifier);
    });

  using Sig = void(std::shared_ptr<CMEnabledState>);
  using Result = boost::asio::async_result<decltype(task), Sig>;
  using Handler = typename Result::completion_handler_type;

  Handler handler(std::forward<decltype(task)>(task));
  Result result(handler);

  auto enabledState = std::make_shared<CMEnabledState>(result.get().share());

  // if this object failed to change state and the current state is DISABLED, try again
  auto succeeded = false;
  std::lock_guard<std::mutex> lk(transitionMutex);
  do {
    succeeded = CompareAndSetState(&currentState, enabledState);
  } while (!succeeded && !currentState->IsEnabled(*this));

  if (succeeded) // succeeded in changing the state
  {
    boost::asio::post(
      threadpool->get_executor(),
      [enabledState, transition = std::move(handler)]() mutable {
        transition(enabledState);
      });
    return enabledState->GetFuture();
  }

  // return the stored future in the current enabled state object
  return currentState->GetFuture();
}

std::shared_future<void> ComponentManagerImpl::PostAsyncEnabledToDisabled(
    std::shared_ptr<cppmicroservices::scrimpl::ComponentManagerState>&
    currentState)
{
  std::packaged_task<void(std::shared_ptr<CMEnabledState>)> task(
    [](std::shared_ptr<CMEnabledState> enabledState) {
      enabledState->DeleteConfigurations();
    });

  using Sig = void(std::shared_ptr<CMEnabledState>);
  using Result = boost::asio::async_result<decltype(task), Sig>;
  using Handler = typename Result::completion_handler_type;

  Handler handler(std::forward<decltype(task)>(task));
  Result result(handler);

  auto disabledState = std::make_shared<CMDisabledState>(result.get().share());

  // if this object failed to change state and the current state is ENABLED, try again
  bool succeeded = false;
  std::lock_guard<std::mutex> lk(transitionMutex);
  do {
    succeeded = CompareAndSetState(&currentState, disabledState);
  } while (!succeeded && currentState->IsEnabled(*this));

  if (succeeded) // succeeded in changing the state
  {
    std::shared_ptr<CMEnabledState> currEnabledState =
      std::dynamic_pointer_cast<CMEnabledState>(currentState);

    boost::asio::post(
      threadpool->get_executor(),
      [currEnabledState, transition = std::move(handler)]() mutable {
        transition(currEnabledState);
      });

    auto fut = disabledState->GetFuture();
    AccumulateFuture(fut);
    return fut;
  }
  // return the stored future in the current disabled state object
  return currentState->GetFuture();
}

}
}
