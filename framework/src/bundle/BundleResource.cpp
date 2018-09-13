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

#include "cppmicroservices/BundleResource.h"

#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/detail/Log.h"

#include "BundleArchive.h"
#include "BundleResourceContainer.h"

#include <atomic>
#include <string>

namespace cppmicroservices {

class BundleResourcePrivate
{

public:
  BundleResourcePrivate(const std::shared_ptr<const BundleArchive>& archive)
    : archive(archive)
  {}

  void InitFilePath(const std::string& file);

  const std::shared_ptr<const BundleArchive> archive;

  BundleResourceContainer::Stat stat;

  std::string fileName;
  std::string path;

  mutable std::vector<std::string> children;
  mutable std::vector<uint32_t> childNodes;
};

void BundleResourcePrivate::InitFilePath(const std::string& file)
{
  std::string normalizedFile = file;
  if (normalizedFile.empty() || normalizedFile[0] != '/') {
    normalizedFile = '/' + normalizedFile;
  }

  std::string rawPath;
  std::size_t index = normalizedFile.find_last_of('/');
  if (index == std::string::npos) {
    fileName = normalizedFile;
  } else if (index < normalizedFile.size() - 1) {
    fileName = normalizedFile.substr(index + 1);
    rawPath = normalizedFile.substr(0, index + 1);
  } else {
    rawPath = normalizedFile;
  }

  // remove duplicate /
  std::string::value_type lastChar = 0;
  for (std::size_t i = 0; i < rawPath.size(); ++i) {
    if (rawPath[i] == '/' && lastChar == '/') {
      continue;
    }
    lastChar = rawPath[i];
    path.push_back(lastChar);
  }
  if (path.empty()) {
    path.push_back('/');
  }
}

BundleResource::BundleResource()
  : data_ptr(std::make_shared<BundleResourcePrivate>(nullptr))
{}

BundleResource::BundleResource(const BundleResource& resource)
  : data_ptr(resource.data_ptr)
{
}

BundleResource::BundleResource(const std::string& file,
                               const std::shared_ptr<const BundleArchive>& archive)
  : data_ptr(std::make_shared<BundleResourcePrivate>(archive))
{
  data_ptr->InitFilePath(file);

  data_ptr->stat.filePath = data_ptr->archive->GetResourcePrefix() + data_ptr->path + data_ptr->fileName;

  data_ptr->archive->GetResourceContainer()->GetStat(data_ptr->stat);
}

BundleResource::BundleResource(int index,
                               const std::shared_ptr<const BundleArchive>& archive)
  : data_ptr(std::make_shared<BundleResourcePrivate>(archive))
{
  data_ptr->archive->GetResourceContainer()->GetStat(index, data_ptr->stat);
  data_ptr->InitFilePath(data_ptr->stat.filePath.substr(data_ptr->archive->GetResourcePrefix().size()));
}

BundleResource::~BundleResource()
{
}

BundleResource& BundleResource::operator=(const BundleResource& resource)
{
  data_ptr = resource.data_ptr;
  return *this;
}

bool BundleResource::operator<(const BundleResource& resource) const
{
  return this->GetResourcePath() < resource.GetResourcePath();
}

bool BundleResource::operator==(const BundleResource& resource) const
{
  if (!this->IsValid())
    return !resource.IsValid();
  if (!resource.IsValid())
    return false;
  return data_ptr->archive->GetResourceContainer() ==
           resource.data_ptr->archive->GetResourceContainer() &&
         data_ptr->archive->GetResourcePrefix() ==
           resource.data_ptr->archive->GetResourcePrefix() &&
         this->GetResourcePath() == resource.GetResourcePath();
}

bool BundleResource::operator!=(const BundleResource& resource) const
{
  return !(*this == resource);
}

bool BundleResource::IsValid() const
{
  return data_ptr->archive && data_ptr->archive->IsValid() && data_ptr->stat.index > -1;
}

BundleResource::operator bool() const
{
  return IsValid();
}

std::string BundleResource::GetName() const
{
  return data_ptr->fileName;
}

std::string BundleResource::GetPath() const
{
  return data_ptr->path;
}

std::string BundleResource::GetResourcePath() const
{
  return data_ptr->path + data_ptr->fileName;
}

std::string BundleResource::GetBaseName() const
{
  return data_ptr->fileName.substr(0, data_ptr->fileName.find_first_of('.'));
}

std::string BundleResource::GetCompleteBaseName() const
{
  return data_ptr->fileName.substr(0, data_ptr->fileName.find_last_of('.'));
}

std::string BundleResource::GetSuffix() const
{
  std::size_t index = data_ptr->fileName.find_last_of('.');
  return index < data_ptr->fileName.size() - 1 ? data_ptr->fileName.substr(index + 1)
                                        : std::string("");
}

std::string BundleResource::GetCompleteSuffix() const
{
  std::size_t index = data_ptr->fileName.find_first_of('.');
  return index < data_ptr->fileName.size() - 1 ? data_ptr->fileName.substr(index + 1)
                                        : std::string("");
}

bool BundleResource::IsDir() const
{
  return data_ptr->stat.isDir;
}

bool BundleResource::IsFile() const
{
  return !data_ptr->stat.isDir;
}

std::vector<std::string> BundleResource::GetChildren() const
{
  if (!IsValid() || !IsDir())
    return data_ptr->children;

  if (data_ptr->children.empty()) {
    data_ptr->archive->GetResourceContainer()->GetChildren(
      data_ptr->stat.filePath, true, data_ptr->children, data_ptr->childNodes);
  }
  return data_ptr->children;
}

std::vector<BundleResource> BundleResource::GetChildResources() const
{
  std::vector<BundleResource> childResources;

  if (!IsValid() || !IsDir())
    return childResources;

  if (data_ptr->childNodes.empty()) {
    data_ptr->archive->GetResourceContainer()->GetChildren(
      this->GetResourcePath(), true, data_ptr->children, data_ptr->childNodes);
  }

  for (std::vector<uint32_t>::const_iterator iter = data_ptr->childNodes.begin(),
                                             iterEnd = data_ptr->childNodes.end();
       iter != iterEnd;
       ++iter) {
    childResources.push_back(
      BundleResource(static_cast<int>(*iter), data_ptr->archive));
  }
  return childResources;
}

int BundleResource::GetSize() const
{
  return data_ptr->stat.uncompressedSize;
}

int BundleResource::GetCompressedSize() const
{
  return data_ptr->stat.compressedSize;
}

time_t BundleResource::GetLastModified() const
{
  return data_ptr->stat.modifiedTime;
}

std::size_t BundleResource::Hash() const
{
  using namespace std;
  return hash<std::string>()(data_ptr->archive->GetResourcePrefix() +
                             this->GetResourcePath());
}

std::unique_ptr<void, void (*)(void*)> BundleResource::GetData() const
{
  if (!IsValid())
    return { nullptr, ::free };

  auto data = data_ptr->archive->GetResourceContainer()->GetData(data_ptr->stat.index);
  if (!data) {
    auto sink = GetBundleContext().GetLogSink();
    DIAG_LOG(*sink) << "Error uncompressing resource data for "
                    << this->GetResourcePath() << " from "
                    << data_ptr->archive->GetBundleLocation();
  }

  return data;
}

std::ostream& operator<<(std::ostream& os, const BundleResource& resource)
{
  return os << resource.GetResourcePath();
}

} // namespace cppmicroservices
