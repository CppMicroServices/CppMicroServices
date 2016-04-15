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

#include "usBundleUtils.h"
#include "usBundleUtils_p.h"

#include <usLog.h>
#include <usBundleInfo_p.h>
#include <usUtils_p.h>

#ifdef __GNUC__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <sys/stat.h>
#if defined(__APPLE__)
#include <sys/param.h>
#include <stdlib.h>
#include <mach-o/dyld.h>
#else
#include <limits.h>
#include <unistd.h>
#endif
#elif _WIN32
#include <windows.h>
#endif



namespace us {

namespace BundleUtils
{

bool IsCurrentExecutable(const std::string& filepath); // forward declaration.

#ifdef __GNUC__

std::string GetLibraryPath_impl(void* symbol)
{
  Dl_info info;
  if (dladdr(symbol, &info))
  {
    return info.dli_fname;
  }
  else
  {
    US_DEBUG << "GetLibraryPath_impl() failed for address " << symbol;
  }
  return "";
}

void* GetSymbol_impl(const BundleInfo& bundleInfo, const char* symbol)
{
  // Clear the last error message
  dlerror();

  void* selfHandle = nullptr;
  
  if (IsCurrentExecutable(bundleInfo.location))
  {
    // Get the handle of the executable
    selfHandle = dlopen(0, RTLD_LAZY);
  }
  else
  {
    selfHandle = dlopen(bundleInfo.location.c_str(), RTLD_LAZY);
  }

  if (selfHandle)
  {
    void* addr = dlsym(selfHandle, symbol);
    if (!addr)
    {
      const char* dlerrorMsg = dlerror();
      if (dlerrorMsg)
      {
        US_DEBUG << "GetSymbol_impl() failed: " << dlerrorMsg;
      }
    }
    dlclose(selfHandle);
    return addr;
  }
  else
  {
    US_DEBUG << "GetSymbol_impl() dlopen() failed: " << dlerror();
  }
  return nullptr;
}
  

#if defined(__APPLE__)
  
std::string GetExecutablePath()
{
  uint32_t bufsize = MAXPATHLEN;
  std::unique_ptr<char[]> buf(new char[bufsize]);
  int status = _NSGetExecutablePath (buf.get(), &bufsize);
  if (status == -1)
  {
    buf.reset(new char[bufsize]);
    status = _NSGetExecutablePath (buf.get(), &bufsize);
  }
  if (status != 0)
  {
    US_DEBUG << "_NSGetExecutablePath() failed";
  }
  // the returned path may not be an absolute path
  return buf.get();
}
  
#else

std::string GetExecutablePath()
{
  char pathBuf[PATH_MAX];
  ssize_t len = readlink("/proc/self/exe", pathBuf, sizeof(pathBuf));
  if (len < 0)
  {
    US_DEBUG << "GetExecutablePath() " << GetLastErrorStr();
    pathBuf[0] = '\0';
  }
  else
  {
    pathBuf[len] = '\0';
  }
  return std::string(pathBuf);
}
  
#endif
  
struct FileInfo
{
  std::string path;
  ino_t stat_ino;
  dev_t stat_dev;
  
  FileInfo() = delete;  // remove default constructor
  
  explicit FileInfo(const std::string& inPath) : path(inPath), stat_ino(0), stat_dev(0)
  {
    if(!path.empty())
    {
      struct stat pathStat;
      if(stat(path.c_str(), &pathStat) == 0)
      {
        stat_ino = pathStat.st_ino;
        stat_dev = pathStat.st_dev;
      }
    }
  }
} execInfo(GetExecutablePath());

inline bool operator==(const FileInfo& lhs, const FileInfo& rhs)
{
  return (lhs.stat_dev == rhs.stat_dev &&
          lhs.stat_ino == rhs.stat_ino);
}
  
#elif _WIN32

std::string GetLibraryPath_impl(void *symbol)
{
  HMODULE handle = nullptr;
  BOOL handleError = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                       static_cast<LPCTSTR>(symbol), &handle);
  if (!handleError)
  {
    // Test
    US_DEBUG << "GetLibraryPath_impl():GetBundleHandle() " << GetLastErrorStr();
    return "";
  }

  char bundlePath[512];
  if (GetModuleFileName(handle, bundlePath, 512))
  {
    return bundlePath;
  }

  US_DEBUG << "GetLibraryPath_impl():GetBundleFileName() " << GetLastErrorStr();
  return "";
}

std::string GetExecutablePath()
{
  char pathBuf[1024]; // assuming this is a large enough buffer.
  if (GetModuleFileName(nullptr, pathBuf, sizeof(pathBuf)) == 0 || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    US_DEBUG << "GetModuleFileName failed" << GetLastErrorStr();
    pathBuf[0] = '\0';
  }
  return std::string(pathBuf);
}
  
struct FileInfo {
  std::string path;
  DWORD dwVolumeSerialNumber;
  DWORD nFileIndexHigh;
  DWORD nFileIndexLow;
  
  FileInfo() = delete; // remove default constructor
  
  explicit FileInfo(const std::string& inPath) : path(inPath), dwVolumeSerialNumber(0), nFileIndexHigh(0), nFileIndexLow(0)
  {
    if(!path.empty())
    {
      HANDLE hFile = CreateFile(path.c_str(),
                                 0,
                                 0,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);
      if(hFile != INVALID_HANDLE_VALUE)
      {
        BY_HANDLE_FILE_INFORMATION fileinfo;
        ZeroMemory(&fileinfo, sizeof(BY_HANDLE_FILE_INFORMATION));
        if(GetFileInformationByHandle(hFile, &fileinfo))
        {
          dwVolumeSerialNumber = fileinfo.dwVolumeSerialNumber;
          nFileIndexHigh = fileinfo.nFileIndexHigh;
          nFileIndexLow = fileinfo.nFileIndexLow;
        }
        CloseHandle(hFile);
      }
      else
      {
        US_DEBUG << "CreateFile() - failed" << GetLastErrorStr();
      }
    }
  }
} execInfo(GetExecutablePath());

inline bool operator==(const FileInfo& lhs, const FileInfo& rhs)
{
  return (lhs.dwVolumeSerialNumber == rhs.dwVolumeSerialNumber &&
          lhs.nFileIndexHigh == rhs.nFileIndexHigh &&
          lhs.nFileIndexLow == rhs.nFileIndexLow);
}

void* GetSymbol_impl(const BundleInfo& bundleInfo, const char* symbol)
{
  HMODULE handle = nullptr;
  
  if (IsCurrentExecutable(bundleInfo.location))
  {
    handle = GetModuleHandle(nullptr);
  }
  else
  {
    handle = GetModuleHandle(bundleInfo.location.c_str());
  }
  
  if (!handle)
  {
    US_DEBUG << "GetSymbol_impl():GetModuleHandle() " << GetLastErrorStr();
    return nullptr;
  }
  
  void* addr = (void*)GetProcAddress(handle, symbol);
  if (!addr)
  {
    US_DEBUG << "GetSymbol_impl():GetProcAddress(handle," << symbol << ") " << GetLastErrorStr();
  }
  return addr;
}

#else

std::string GetLibraryPath_impl(void*)
{
  return "";
}

void* GetSymbol_impl(const BundleInfo&, const char* symbol)
{
  return nullptr;
}

#endif

std::string GetLibraryPath(void* symbol)
{
  return GetLibraryPath_impl(symbol);
}

void* GetSymbol(const std::string& bundleName, const std::string& libLocation, const char* symbol)
{
  BundleInfo info(bundleName);
  info.location = libLocation;
  return GetSymbol(info, symbol);
}

void* GetSymbol(const BundleInfo& bundle, const char* symbol)
{
  return GetSymbol_impl(bundle, symbol);
}
  
bool IsCurrentExecutable(const std::string& filepath)
{
  // If two path strings are equal they both refer to the same file. Even if the
  // path strings are not equal, they may refer to the same file due to simlinks,
  // relative paths etc. Hence we use the FileInfo object to compare the file
  // identifiers
  // Optimization: Avoid constructing the FileInfo object if strings match
  return execInfo.path == filepath || execInfo == FileInfo(filepath);
}

} // namespace BundleUtils

} // namespace us
