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

#ifndef CPPMICROSERVICES_BUNDLESTORAGEMEMORY_H
#define CPPMICROSERVICES_BUNDLESTORAGEMEMORY_H

#include "cppmicroservices/detail/Threads.h"

#include "BundleStorage.h"

#include <map>
#include <memory>

namespace cppmicroservices
{

    class BundleStorageMemory : public BundleStorage
    {

      public:
        BundleStorageMemory();

        std::shared_ptr<BundleArchive> CreateAndInsertArchive(std::shared_ptr<BundleResourceContainer> const& resCont,
                                                              std::string const& topLevelEntry,
                                                              ManifestT const& bundleManifest) override;

        bool RemoveArchive(BundleArchive const* ba) override;

        std::vector<std::shared_ptr<BundleArchive>> GetAllBundleArchives() const override;

        std::vector<long> GetStartOnLaunchBundles() const override;

        void Close() override;

      private:
        long nextFreeId;
        /**
         * Bundle id sorted list of all active bundle archives.
         */
        struct : detail::MultiThreaded<>
        {
            std::map<long, std::shared_ptr<BundleArchive>> v;
        } archives;
    };

} // namespace cppmicroservices

#endif // CPPMICROSERVICES_BUNDLESTORAGEMEMORY_H
