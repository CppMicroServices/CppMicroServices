/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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

#ifndef USMODULERESOURCECONTAINER_P_H
#define USMODULERESOURCECONTAINER_P_H

#include "usGlobalConfig.h"
#include "us_stdint.h"

#include <ctime>
#include <string>
#include <vector>

US_BEGIN_NAMESPACE

struct ModuleInfo;
class ModuleResource;
struct ModuleResourceContainerPrivate;

class ModuleResourceContainer
{

public:

  ModuleResourceContainer(const ModuleInfo* moduleInfo);
  ~ModuleResourceContainer();

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

  const ModuleInfo* GetModuleInfo() const;

  void GetChildren(const std::string& resourcePath, bool relativePaths,
                   std::vector<std::string>& names, std::vector<uint32_t>& indices) const;

  void FindNodes(const std::string& path, const std::string& filePattern,
                 bool recurse, std::vector<ModuleResource>& resources) const;

private:

  bool Matches(const std::string& name, const std::string& filePattern) const;

  ModuleResourceContainerPrivate* d;

};


US_END_NAMESPACE

#endif // USMODULERESOURCECONTAINER_P_H
