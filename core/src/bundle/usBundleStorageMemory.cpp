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

#include "usBundleStorageMemory_p.h"

#include "usBundleArchive_p.h"
#include "usBundleResourceContainer_p.h"

namespace us {

BundleStorageMemory::BundleStorageMemory()
  : nextFreeId(1)
{
}

std::vector<std::shared_ptr<BundleArchive>> BundleStorageMemory::InsertBundleLib(const std::string& location)
{
  auto resCont = std::make_shared<BundleResourceContainer>(location);
  return InsertArchives(resCont, resCont->GetTopLevelDirs());
}

std::vector<std::shared_ptr<BundleArchive>> BundleStorageMemory::InsertArchives(
    const std::shared_ptr<const BundleResourceContainer>& resCont,
    const std::vector<std::string>& topLevelEntries)
{
  std::vector<std::shared_ptr<BundleArchive>> res;
  for (auto const& prefix : topLevelEntries)
  {
    auto id = nextFreeId++;
    std::unique_ptr<BundleArchive::Data> data(new BundleArchive::Data{id, 0, -1});
    auto p = archives.v.insert(std::make_pair(
          id,
          std::make_shared<BundleArchive>(this, std::move(data), resCont, prefix, resCont->GetLocation())
          ));
    res.push_back(p.first->second);
  }
  return res;
}

bool BundleStorageMemory::RemoveArchive(const BundleArchive* ba)
{
  auto l = archives.Lock(); US_UNUSED(l);
  auto iter = archives.v.find(ba->GetBundleId());
  if (iter != archives.v.end())
  {
    archives.v.erase(iter);
    return true;
  }
  return false;
}

std::vector<std::shared_ptr<BundleArchive>> BundleStorageMemory::GetAllBundleArchives() const
{
  std::vector<std::shared_ptr<BundleArchive>> res;
  auto l = archives.Lock(); US_UNUSED(l);
  for (auto const& v : archives.v)
  {
    res.emplace_back(v.second);
  }
  return res;
}

std::vector<long> BundleStorageMemory::GetStartOnLaunchBundles() const
{
  std::vector<long> res;
  for (auto& v : archives.v)
  {
    if (v.second->GetAutostartSetting() != -1)
    {
      res.emplace_back(v.second->GetBundleId());
    }
  }
  return res;
}

void BundleStorageMemory::Close()
{
  std::map<long, std::shared_ptr<BundleArchive>> ar;
  {
    archives.Lock();
    ar = std::move(archives.v);
    archives.v.clear();
  }
  ar.clear();
}

}
