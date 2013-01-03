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


#ifndef USMODULERESOURCETREE_H
#define USMODULERESOURCETREE_H

#include <usConfig.h>

#include "stdint_p.h"

#include <vector>

US_BEGIN_NAMESPACE

struct ModuleInfo;

class ModuleResourceTree
{

private:

  enum Flags
  {
    Directory = 0x01
  };

  bool isValid;
  const unsigned char *tree, *names, *payloads;

  // Returns the offset in the us_resource_tree array for a given node index
  inline int FindOffset(int node) const { return node * 14; } // sizeof each tree element

  std::string GetName(int node) const;
  short GetFlags(int node) const;

  void FindNodes(std::vector<std::string>& result, const std::string& path,
                 const int rootNode,
                 const std::string& filePattern, bool recurse);

  uint32_t GetHash(int node) const;

  bool Matches(const std::string& name, const std::string& filePattern);

public:

  ModuleResourceTree(const ModuleInfo* moduleInfo);

  bool IsValid() const;
  void Invalidate();

  // Returns an index enumerating the info entries in the us_resource_tree array for
  // the given resource path.
  int FindNode(const std::string& path) const;

  void FindNodes(const std::string& path, const std::string& filePattern,
                 bool recurse, std::vector<std::string>& result);

  inline bool IsDir(int node) const { return GetFlags(node) & Directory; }
  const unsigned char* GetData(int node, int32_t *size) const;
  void GetChildren(int node, std::vector<std::string>& children) const;

};

US_END_NAMESPACE

#endif // USMODULERESOURCETREE_H
