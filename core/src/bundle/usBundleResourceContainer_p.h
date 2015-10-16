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

#ifndef USBUNDLERESOURCECONTAINER_P_H
#define USBUNDLERESOURCECONTAINER_P_H

#include "usGlobalConfig.h"

#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

namespace us {

struct BundleInfo;
class BundleResource;
struct BundleResourceContainerPrivate;

class BundleResourceContainer
{

public:

  BundleResourceContainer(const BundleInfo* bundleInfo);
  ~BundleResourceContainer();

  struct Stat
  {
    Stat()
      : index(-1)
      , uncompressedSize(0)
      , modifiedTime(0)
      , isDir(false)
    {}

    std::string filePath;
    int index;
    int uncompressedSize;
    time_t modifiedTime;
    bool isDir;
  };

  bool IsValid() const;

  bool GetStat(Stat& stat) const;
  bool GetStat(int index, Stat& stat) const;

  void* GetData(int index) const;

  const BundleInfo* GetBundleInfo() const;

  void GetChildren(const std::string& resourcePath, bool relativePaths,
                   std::vector<std::string>& names, std::vector<uint32_t>& indices) const;

  void FindNodes(const std::string& path, const std::string& filePattern,
                 bool recurse, std::vector<BundleResource>& resources) const;

private:

  bool Matches(const std::string& name, const std::string& filePattern) const;

  BundleResourceContainerPrivate* d;

};


}

#endif // USBUNDLERESOURCECONTAINER_P_H
