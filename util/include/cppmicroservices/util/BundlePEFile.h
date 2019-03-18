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

#if defined (US_PLATFORM_WINDOWS)

#include "BundleObjFile.h"
#include "DataContainer.h"
#include "Error.h"
#include "cppmicroservices_pe.h"

#include <cerrno>
#include <fstream>
#include <memory>

#include <sys/stat.h>

#include <Windows.h>

namespace cppmicroservices {

struct hModuleDeleter
{
  // if the unique_ptr's deleter contains a nested type named 'pointer',
  // then the unique_ptr will use that type for its managed object
  // pointer instead of T*.
  typedef HMODULE pointer;

  void operator()(HMODULE handle) const {
    if (handle) {
      FreeLibrary(handle);
    }
  }
};

// A DataContainer which scopes the lifetimes of the
// Windows DLL and the data within it together.
class ResDataContainer final : public DataContainer
{
public:
  ResDataContainer(std::unique_ptr<HMODULE, hModuleDeleter> moduleHandle,
                   LPVOID data,
                   DWORD dataSize)
    : m_ModuleHandle(std::move(moduleHandle))
    , m_Data(data)
    , m_DataSize(dataSize)
  {}
  ~ResDataContainer() = default;

  void* GetData() const override { return m_Data; }
  std::size_t GetSize() const override { return m_DataSize; }

private:
  std::unique_ptr<HMODULE, hModuleDeleter> m_ModuleHandle;
  LPVOID m_Data;
  DWORD m_DataSize;
};

struct InvalidPEException : public InvalidObjFileException
{
  InvalidPEException(std::string what, int errorNumber = 0)
    : InvalidObjFileException(std::move(what), errorNumber)
  {}
};

class BundlePEFile : public BundleObjFile
{
public:
  BundlePEFile(std::string location)
    : m_rawData(nullptr)
  {
    HMODULE hBundleResources = LoadLibraryEx(location.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    if (hBundleResources) {
      // RAII - automatically free the library handle on object destruction
      auto loadLibraryHandle = std::unique_ptr<HMODULE, hModuleDeleter>(hBundleResources, hModuleDeleter{});
      HRSRC hResource = FindResource(hBundleResources, "US_RESOURCES", MAKEINTRESOURCE(300));
      HGLOBAL hRes = LoadResource(hBundleResources, hResource);
      const LPVOID res = LockResource(hRes);
      const DWORD zipSizeInBytes = SizeofResource(hBundleResources, hResource);
      if (0 < zipSizeInBytes) {
        m_rawData = std::make_shared<RawBundleResources>(std::make_unique<ResDataContainer>(std::move(loadLibraryHandle), res, zipSizeInBytes));
      }
    }
    else {
      throw InvalidPEException("LoadLibrary failed for path " + location + " with error " + cppmicroservices::util::GetLastWin32ErrorStr());
    }
  };
  ~BundlePEFile() = default;

  std::shared_ptr<RawBundleResources> GetRawBundleResourceContainer() const override
  {
    return m_rawData;
  }

private:
  std::shared_ptr<RawBundleResources> m_rawData;
};

std::unique_ptr<BundleObjFile> CreateBundlePEFile(const std::string& fileName)
{
  struct stat peStat;
  errno = 0;
  if (stat(fileName.c_str(), &peStat) != 0) {
    throw InvalidPEException("Stat for " + fileName + " failed", errno);
  }
  return std::make_unique<BundlePEFile>(fileName);
}

}

#endif
