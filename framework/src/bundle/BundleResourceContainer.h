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

#ifndef CPPMICROSERVICES_BUNDLERESOURCECONTAINER_H
#define CPPMICROSERVICES_BUNDLERESOURCECONTAINER_H

#include "miniz.h"

#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <mutex>

namespace cppmicroservices {

struct BundleArchive;
class BundleResource;

class BundleResourceContainer : public std::enable_shared_from_this<BundleResourceContainer>
{

public:

  BundleResourceContainer(const std::string& location);
  ~BundleResourceContainer();

  struct Stat
  {
    Stat()
      : index(-1)
      , compressedSize(0)
      , uncompressedSize(0)
      , modifiedTime(0)
      , isDir(false)
    {}

    std::string filePath;
    int index;
    int compressedSize;
    int uncompressedSize;
    time_t modifiedTime;
    bool isDir;
  };

  std::string GetLocation() const;

  std::vector<std::string> GetTopLevelDirs() const;

  bool GetStat(Stat& stat) const;
  bool GetStat(int index, Stat& stat) const;

  std::unique_ptr<void, void(*)(void*)> GetData(int index) const;

  void GetChildren(const std::string& resourcePath, bool relativePaths,
                   std::vector<std::string>& names, std::vector<uint32_t>& indices) const;

  void FindNodes(const std::shared_ptr<const BundleArchive>& archive, const std::string& path, const std::string& filePattern,
                 bool recurse, std::vector<BundleResource>& resources) const;

private:

  typedef std::pair<std::string, int> NameIndexPair;

  struct PairComp
  {
    inline bool operator()(const NameIndexPair& p1, const NameIndexPair& p2) const
    {
      return p1.first < p2.first;
    }
  };

  void InitSortedEntries();

  bool Matches(const std::string& name, const std::string& filePattern) const;

  const std::string m_Location;
  mz_zip_archive m_ZipArchive;

  std::set<NameIndexPair, PairComp> m_SortedEntries;
  std::set<std::string> m_SortedToplevelDirs;

  // This is used to synchronize miniz file stream API calls.
  // Working with file streams is stateful (e.g. current read position)
  // and hence not thread-safe.
  mutable std::mutex m_ZipFileStreamMutex;
};

}

#endif // CPPMICROSERVICES_BUNDLERESOURCECONTAINER_H
