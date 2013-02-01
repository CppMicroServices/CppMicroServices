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


#ifndef USMODULERESOURCE_H
#define USMODULERESOURCE_H

#include <usConfig.h>

#include <ostream>
#include <vector>

US_MSVC_PUSH_DISABLE_WARNING(4396)

US_BEGIN_NAMESPACE

class ModuleResourcePrivate;
class ModuleResourceTree;

class US_EXPORT ModuleResource
{

public:

  ModuleResource();
  ModuleResource(const ModuleResource& resource);

  ~ModuleResource();

  ModuleResource& operator=(const ModuleResource& resource);

  bool operator<(const ModuleResource& resource) const;
  bool operator==(const ModuleResource& resource) const;
  bool operator!=(const ModuleResource& resource) const;

  bool IsValid() const;
  operator bool() const;

  std::string GetName() const;
  std::string GetPath() const;
  std::string GetResourcePath() const;

  std::string GetBaseName() const;
  std::string GetCompleteBaseName() const;
  std::string GetSuffix() const;

  bool IsDir() const;
  bool IsFile() const;

  std::vector<std::string> GetChildren() const;

  int GetSize() const;
  const unsigned char* GetData() const;

private:

  ModuleResource(const std::string& file, ModuleResourceTree* resourceTree,
                 const std::vector<ModuleResourceTree*>& resourceTrees);

  friend class Module;

  US_HASH_FUNCTION_FRIEND(ModuleResource);

  std::size_t Hash() const;

  ModuleResourcePrivate* d;

};

US_END_NAMESPACE

US_MSVC_POP_WARNING

US_EXPORT std::ostream& operator<<(std::ostream& os, const US_PREPEND_NAMESPACE(ModuleResource)& resource);

US_HASH_FUNCTION_NAMESPACE_BEGIN
US_HASH_FUNCTION_BEGIN(US_PREPEND_NAMESPACE(ModuleResource))
  return arg.Hash();
US_HASH_FUNCTION_END
US_HASH_FUNCTION_NAMESPACE_END

#endif // USMODULERESOURCE_H
