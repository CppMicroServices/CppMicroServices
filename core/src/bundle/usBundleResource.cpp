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


#include "usBundleResource.h"

#include "usAtomicInt_p.h"
#include "usBundleResourceContainer_p.h"
#include "usBundleInfo.h"
#include "usLog_p.h"

#include <string>

US_BEGIN_NAMESPACE

class BundleResourcePrivate
{

public:

  BundleResourcePrivate(const BundleResourceContainer* rc)
    : resourceContainer(rc)
    , ref(1)
  {}

  void InitFilePath(const std::string& file);

  const BundleResourceContainer* const resourceContainer;

  BundleResourceContainer::Stat stat;

  std::string fileName;
  std::string path;

  mutable std::vector<std::string> children;
  mutable std::vector<uint32_t> childNodes;

  /**
   * Reference count for implicitly shared private implementation.
   */
  AtomicInt ref;
};

void BundleResourcePrivate::InitFilePath(const std::string& file)
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

BundleResource::BundleResource()
  : d(new BundleResourcePrivate(NULL))
{
}

BundleResource::BundleResource(const BundleResource &resource)
  : d(resource.d)
{
  d->ref.Ref();
}

BundleResource::BundleResource(const std::string& file, const BundleResourceContainer& resourceContainer)
  : d(new BundleResourcePrivate(&resourceContainer))
{
  d->InitFilePath(file);

  d->stat.filePath = d->resourceContainer->GetBundleInfo()->name + d->path + d->fileName;

  d->resourceContainer->GetStat(d->stat);
}

BundleResource::BundleResource(int index, const BundleResourceContainer& resourceContainer)
  : d(new BundleResourcePrivate(&resourceContainer))
{
  d->resourceContainer->GetStat(index, d->stat);
  d->InitFilePath(d->stat.filePath.substr(d->resourceContainer->GetBundleInfo()->name.size()));
}

BundleResource::~BundleResource()
{
  if (!d->ref.Deref())
    delete d;
}

BundleResource& BundleResource::operator =(const BundleResource& resource)
{
  BundleResourcePrivate* curr_d = d;
  d = resource.d;
  d->ref.Ref();

  if (!curr_d->ref.Deref())
    delete curr_d;

  return *this;
}

bool BundleResource::operator <(const BundleResource& resource) const
{
  return this->GetResourcePath() < resource.GetResourcePath();
}

bool BundleResource::operator ==(const BundleResource& resource) const
{
  return d->resourceContainer == resource.d->resourceContainer &&
      this->GetResourcePath() == resource.GetResourcePath();
}

bool BundleResource::operator !=(const BundleResource &resource) const
{
  return !(*this == resource);
}

bool BundleResource::IsValid() const
{
  return d->resourceContainer && d->resourceContainer->IsValid() && d->stat.index > -1;
}

BundleResource::operator bool_type() const
{
  return IsValid() ? &BundleResource::d : NULL;
}

std::string BundleResource::GetName() const
{
  return d->fileName;
}

std::string BundleResource::GetPath() const
{
  return d->path;
}

std::string BundleResource::GetResourcePath() const
{
  return d->path + d->fileName;
}

std::string BundleResource::GetBaseName() const
{
  return d->fileName.substr(0, d->fileName.find_first_of('.'));
}

std::string BundleResource::GetCompleteBaseName() const
{
  return d->fileName.substr(0, d->fileName.find_last_of('.'));
}

std::string BundleResource::GetSuffix() const
{
  std::size_t index = d->fileName.find_last_of('.');
  return index < d->fileName.size()-1 ? d->fileName.substr(index+1) : std::string("");
}

std::string BundleResource::GetCompleteSuffix() const
{
  std::size_t index = d->fileName.find_first_of('.');
  return index < d->fileName.size()-1 ? d->fileName.substr(index+1) : std::string("");
}

bool BundleResource::IsDir() const
{
  return d->stat.isDir;
}

bool BundleResource::IsFile() const
{
  return !d->stat.isDir;
}

std::vector<std::string> BundleResource::GetChildren() const
{
  if (!IsValid() || !IsDir()) return d->children;

  if (d->children.empty())
  {
    d->resourceContainer->GetChildren(d->stat.filePath, true,
                                      d->children, d->childNodes);
  }
  return d->children;
}

std::vector<BundleResource> BundleResource::GetChildResources() const
{
  std::vector<BundleResource> childResources;

  if (!IsValid() || !IsDir()) return childResources;

  if (d->childNodes.empty())
  {
    d->resourceContainer->GetChildren(this->GetResourcePath(), true,
                                      d->children, d->childNodes);
  }

  for (std::vector<uint32_t>::const_iterator iter = d->childNodes.begin(),
       iterEnd = d->childNodes.end(); iter != iterEnd; ++iter)
  {
    childResources.push_back(BundleResource(static_cast<int>(*iter), *d->resourceContainer));
  }
  return childResources;
}

int BundleResource::GetSize() const
{
  return d->stat.uncompressedSize;
}

time_t BundleResource::GetLastModified() const
{
  return d->stat.modifiedTime;
}

std::size_t BundleResource::Hash() const
{
  using namespace US_HASH_FUNCTION_NAMESPACE;
  return US_HASH_FUNCTION(std::string, d->resourceContainer->GetBundleInfo()->name + this->GetResourcePath());
}

void* BundleResource::GetData() const
{
  if (!IsValid()) return NULL;

  void* data = d->resourceContainer->GetData(d->stat.index);
  if (data == NULL)
  {
    US_WARN << "Error uncompressing resource data for " << this->GetResourcePath() << " from " 
            << d->resourceContainer->GetBundleInfo()->location;
  }
  return data;
}

US_END_NAMESPACE

US_USE_NAMESPACE

std::ostream& operator<<(std::ostream& os, const BundleResource& resource)
{
  return os << resource.GetResourcePath();
}
