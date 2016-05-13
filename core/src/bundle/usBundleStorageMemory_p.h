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

#ifndef USBUNDLESTORAGEMEMORY_P_H
#define USBUNDLESTORAGEMEMORY_P_H

#include "usBundleStorage_p.h"
#include "usThreads_p.h"

#include <map>
#include <memory>

namespace us {

class BundleStorageMemory : public BundleStorage
{

public:

  BundleStorageMemory();

  std::vector<std::shared_ptr<BundleArchive>> InsertBundleLib(const std::string& location);

  std::vector<std::shared_ptr<BundleArchive>> InsertArchives(
      const std::shared_ptr<const BundleResourceContainer>& resCont,
      const std::vector<std::string>& topLevelEntries);

  bool RemoveArchive(const BundleArchive* ba);

  std::vector<std::shared_ptr<BundleArchive>> GetAllBundleArchives() const;

  std::vector<long> GetStartOnLaunchBundles() const;

  void Close();

private:

  /**
   * Next available bundle id.
   */
  long nextFreeId;

  /**
   * Bundle id sorted list of all active bundle archives.
   */
  struct : MultiThreaded<> { std::map<long, std::shared_ptr<BundleArchive>> v; } archives;


};

}

#endif // USBUNDLESTORAGEMEMORY_P_H
