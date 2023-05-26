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

#include "BundleStorageMemory.h"
#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/Constants.h"

#include "BundleArchive.h"
#include "BundleResourceContainer.h"

#include <chrono>

namespace cppmicroservices
{
    namespace sc = std::chrono;

    BundleStorageMemory::BundleStorageMemory() : BundleStorage(), nextFreeId(1) {}

    std::shared_ptr<BundleArchive>
    BundleStorageMemory::CreateAndInsertArchive(std::shared_ptr<BundleResourceContainer> const& resCont,
                                                std::string const& prefix,
                                                ManifestT const& bundleManifest)
    {
        auto l = archives.Lock();
        US_UNUSED(l);
        auto id = nextFreeId++;
        auto p = archives.v.insert(std::make_pair(
            id,
            std::make_shared<BundleArchive>(this, resCont, prefix, resCont->GetLocation(), id, bundleManifest)));
        return p.first->second;
    }

    bool
    BundleStorageMemory::RemoveArchive(BundleArchive const* ba)
    {
        auto l = archives.Lock();
        US_UNUSED(l);
        auto iter = archives.v.find(ba->GetBundleId());
        if (iter != archives.v.end())
        {
            archives.v.erase(iter);
            return true;
        }
        return false;
    }

    std::vector<std::shared_ptr<BundleArchive>>
    BundleStorageMemory::GetAllBundleArchives() const
    {
        std::vector<std::shared_ptr<BundleArchive>> res;
        auto l = archives.Lock();
        US_UNUSED(l);
        for (auto const& v : archives.v)
        {
            res.emplace_back(v.second);
        }
        return res;
    }

    std::vector<long>
    BundleStorageMemory::GetStartOnLaunchBundles() const
    {
        std::vector<long> res;
        auto l = archives.Lock();
        US_UNUSED(l);
        for (auto& v : archives.v)
        {
            if (v.second->GetAutostartSetting() != -1)
            {
                res.emplace_back(v.second->GetBundleId());
            }
        }
        return res;
    }

    void
    BundleStorageMemory::Close()
    {
        auto l = archives.Lock();
        US_UNUSED(l);
        archives.v.clear();
    }
} // namespace cppmicroservices
