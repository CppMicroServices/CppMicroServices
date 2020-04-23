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
#include "cppmicroservices/SharedLibraryException.h"
#include "states/CMDisabledState.hpp"
#include "states/ComponentManagerState.hpp"
#include <cassert>
#include <utility>

namespace cppmicroservices {
namespace scrimpl {

ComponentManagerImpl::ComponentManagerImpl(std::shared_ptr<const metadata::ComponentMetadata> metadata,
                                           std::shared_ptr<const ComponentRegistry> registry,
                                           BundleContext bundleContext,
                                           std::shared_ptr<cppmicroservices::logservice::LogService> logger)
  : registry(std::move(registry))
  , compDesc(std::move(metadata))
  , bundleContext(std::move(bundleContext))
  , logger(std::move(logger))
  , state(std::make_shared<CMDisabledState>())
{
  if(!compDesc || !this->registry || !this->bundleContext || !this->logger)
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
}
}
