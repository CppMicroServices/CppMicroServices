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

#include "ComponentRegistry.hpp"

namespace cppmicroservices {
namespace scrimpl {

std::vector<std::shared_ptr<ComponentManager>> ComponentRegistry::GetComponentManagers() const
{
  std::lock_guard<std::mutex> lock(mMapsMutex); 
  std::vector<std::shared_ptr<ComponentManager>> managers;
  for (const auto& kv : mComponentsByName)
  {
    managers.push_back(kv.second);
  }
  return managers;
}

std::vector<std::shared_ptr<ComponentManager>> ComponentRegistry::GetComponentManagers(unsigned long bundleId) const
{
  std::lock_guard<std::mutex> lock(mMapsMutex);
  std::vector<std::shared_ptr<ComponentManager>> managers;
  for (const auto& kv : mComponentsByName)
  {
    if(kv.first.first == bundleId)
    {
      managers.push_back(kv.second);
    }
  }
  return managers;
}

std::shared_ptr<ComponentManager> ComponentRegistry::GetComponentManager(unsigned long bundleId,
                                                                         const std::string& compName) const
{
  std::lock_guard<std::mutex> lock(mMapsMutex);
  return mComponentsByName.at(std::make_pair(bundleId, compName));
}

bool ComponentRegistry::AddComponentManager(const std::shared_ptr<ComponentManager>& cm)
{
  std::lock_guard<std::mutex> lock(mMapsMutex);
  auto result = mComponentsByName.insert(std::make_pair(std::make_pair(static_cast<unsigned long>(cm->GetBundleId()), cm->GetName()), cm));
  return result.second;
}

void ComponentRegistry::RemoveComponentManager(unsigned long bundleId,
                                               const std::string& compName)
{
  std::lock_guard<std::mutex> lock(mMapsMutex);
  mComponentsByName.erase(std::make_pair(bundleId, compName));
}

void ComponentRegistry::RemoveComponentManager(const std::shared_ptr<ComponentManager>& cm)
{
  RemoveComponentManager(cm->GetBundleId(),
                         cm->GetName());
}

void ComponentRegistry::Clear()
{
  std::lock_guard<std::mutex> lock(mMapsMutex);
  mComponentsByName.clear();
}

size_t ComponentRegistry::Count() const
{
  std::lock_guard<std::mutex> lock(mMapsMutex);
  return mComponentsByName.size();
}
}
}
