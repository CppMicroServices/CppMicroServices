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


#include "usModuleResource.h"

#include "usAtomicInt_p.h"
#include "usModuleResourceContainer_p.h"
#include "usModuleInfo.h"

#include <string>

US_BEGIN_NAMESPACE

class ModuleResourcePrivate
{

public:

  ModuleResourcePrivate(const ModuleResourceContainer* rc)
    : resourceContainer(rc)
    , ref(1)
  {}

  void InitFilePath(const std::string& file);

  const ModuleResourceContainer* const resourceContainer;

  ModuleResourceContainer::Stat stat;

  std::string fileName;
  std::string path;

  mutable std::vector<std::string> children;
  mutable std::vector<uint32_t> childNodes;

  /**
   * Reference count for implicitly shared private implementation.
   */
  AtomicInt ref;
};

void ModuleResourcePrivate::InitFilePath(const std::string& file)
{
  std::string normalizedFile = file;
  if (normalizedFile.empty() || normalizedFile[0] != '/')
  {
    normalizedFile = '/' + normalizedFile;
  }

  std::string rawPath;
  std::size_t index = normalizedFile.find_last_of('/');
  if (index == std::string::npos)
  {
    fileName = normalizedFile;
  }
  else if (index < normalizedFile.size()-1)
  {
    fileName = normalizedFile.substr(index+1);
    rawPath = normalizedFile.substr(0,index+1);
  }
  else
  {
    rawPath = normalizedFile;
  }

  // remove duplicate /
  std::string::value_type lastChar = 0;
  for (std::size_t i = 0; i < rawPath.size(); ++i)
  {
    if (rawPath[i] == '/' && lastChar == '/')
    {
      continue;
    }
    lastChar = rawPath[i];
    path.push_back(lastChar);
  }
  if (path.empty())
  {
    path.push_back('/');
  }
}

ModuleResource::ModuleResource()
  : d(new ModuleResourcePrivate(NULL))
{
}

ModuleResource::ModuleResource(const ModuleResource &resource)
  : d(resource.d)
{
  d->ref.Ref();
}

ModuleResource::ModuleResource(const std::string& file, const ModuleResourceContainer& resourceContainer)
  : d(new ModuleResourcePrivate(&resourceContainer))
{
  d->InitFilePath(file);

  d->stat.filePath = d->resourceContainer->GetModuleInfo()->name + d->path + d->fileName;

  d->resourceContainer->GetStat(d->stat);
}

ModuleResource::ModuleResource(int index, const ModuleResourceContainer& resourceContainer)
  : d(new ModuleResourcePrivate(&resourceContainer))
{
  d->resourceContainer->GetStat(index, d->stat);
  d->InitFilePath(d->stat.filePath.substr(d->resourceContainer->GetModuleInfo()->name.size()));
}

ModuleResource::~ModuleResource()
{
  if (!d->ref.Deref())
    delete d;
}

ModuleResource& ModuleResource::operator =(const ModuleResource& resource)
{
  ModuleResourcePrivate* curr_d = d;
  d = resource.d;
  d->ref.Ref();

  if (!curr_d->ref.Deref())
    delete curr_d;

  return *this;
}

bool ModuleResource::operator <(const ModuleResource& resource) const
{
  return this->GetResourcePath() < resource.GetResourcePath();
}

bool ModuleResource::operator ==(const ModuleResource& resource) const
{
  return d->resourceContainer == resource.d->resourceContainer &&
      this->GetResourcePath() == resource.GetResourcePath();
}

bool ModuleResource::operator !=(const ModuleResource &resource) const
{
  return !(*this == resource);
}

bool ModuleResource::IsValid() const
{
  return d->resourceContainer && d->resourceContainer->IsValid() && d->stat.index > -1;
}

ModuleResource::operator bool_type() const
{
  return IsValid() ? &ModuleResource::d : NULL;
}

std::string ModuleResource::GetName() const
{
  return d->fileName;
}

std::string ModuleResource::GetPath() const
{
  return d->path;
}

std::string ModuleResource::GetResourcePath() const
{
  return d->path + d->fileName;
}

std::string ModuleResource::GetBaseName() const
{
  return d->fileName.substr(0, d->fileName.find_first_of('.'));
}

std::string ModuleResource::GetCompleteBaseName() const
{
  return d->fileName.substr(0, d->fileName.find_last_of('.'));
}

std::string ModuleResource::GetSuffix() const
{
  std::size_t index = d->fileName.find_last_of('.');
  return index < d->fileName.size()-1 ? d->fileName.substr(index+1) : std::string("");
}

std::string ModuleResource::GetCompleteSuffix() const
{
  std::size_t index = d->fileName.find_first_of('.');
  return index < d->fileName.size()-1 ? d->fileName.substr(index+1) : std::string("");
}

bool ModuleResource::IsDir() const
{
  return d->stat.isDir;
}

bool ModuleResource::IsFile() const
{
  return !d->stat.isDir;
}

std::vector<std::string> ModuleResource::GetChildren() const
{
  if (!IsValid() || !IsDir()) return d->children;

  if (d->children.empty())
  {
    d->resourceContainer->GetChildren(d->stat.filePath, true,
                                      d->children, d->childNodes);
  }
  return d->children;
}

std::vector<ModuleResource> ModuleResource::GetChildResources() const
{
  std::vector<ModuleResource> childResources;

  if (!IsValid() || !IsDir()) return childResources;

  if (d->childNodes.empty())
  {
    d->resourceContainer->GetChildren(this->GetResourcePath(), true,
                                      d->children, d->childNodes);
  }

  for (std::vector<uint32_t>::const_iterator iter = d->childNodes.begin(),
       iterEnd = d->childNodes.end(); iter != iterEnd; ++iter)
  {
    childResources.push_back(ModuleResource(static_cast<int>(*iter), *d->resourceContainer));
  }
  return childResources;
}

int ModuleResource::GetSize() const
{
  return d->stat.uncompressedSize;
}

time_t ModuleResource::GetLastModified() const
{
  return d->stat.modifiedTime;
}

std::size_t ModuleResource::Hash() const
{
  using namespace US_HASH_FUNCTION_NAMESPACE;
  return US_HASH_FUNCTION(std::string, d->resourceContainer->GetModuleInfo()->name + this->GetResourcePath());
}

void* ModuleResource::GetData() const
{
  if (!IsValid()) return NULL;

  void* data = d->resourceContainer->GetData(d->stat.index);
  if (data == NULL)
  {
    US_WARN << "Error uncompressing resource data for " << this->GetResourcePath() << " from "
            << d->resourceContainer->GetModuleInfo()->location;
  }
  return data;
}

US_END_NAMESPACE

US_USE_NAMESPACE

std::ostream& operator<<(std::ostream& os, const ModuleResource& resource)
{
  return os << resource.GetResourcePath();
}
