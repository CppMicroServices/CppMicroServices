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

#include "BundleStorageFile.h"

#include "BundleArchive.h"

#include <stdexcept>

namespace cppmicroservices {

BundleStorageFile::BundleStorageFile()
  : BundleStorage()
{
  throw std::logic_error("not implemented");
}

std::shared_ptr<BundleArchive> BundleStorageFile::CreateAndInsertArchive(
  const std::shared_ptr<BundleResourceContainer>& /*resCont*/
  ,
  const std::string& /*topLevelEntries*/
  ,
  const ManifestT&)
{
  throw std::logic_error("not implemented");
}

bool BundleStorageFile::RemoveArchive(const BundleArchive* /*ba*/)
{
  throw std::logic_error("not implemented");
}

std::vector<std::shared_ptr<BundleArchive>>
BundleStorageFile::GetAllBundleArchives() const
{
  throw std::logic_error("not implemented");
}

std::vector<long> BundleStorageFile::GetStartOnLaunchBundles() const
{
  throw std::logic_error("not implemented");
}

void BundleStorageFile::Close()
{
  throw std::logic_error("not implemented");
}
}
