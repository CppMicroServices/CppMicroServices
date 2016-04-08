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
#include <usUtils_p.h>

#ifdef __GNUC__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#elif _WIN32
#include <windows.h>
#endif

namespace us {

/**
 * A helper class to capture error code information per function
 * call.
 * Internal use only.
 *
 * Functionally equivalent to the free standing BundleUtils
 * functions.
 *
 * Example usage
 * <code>
 * BundleUtilWrapper util;
 * std::string path(util.GetLibraryPath(symbol));
 * if(path.empty())
 * {  
 *   std::string err(util.LastError());
 *   // ... do something with error string ...
 * }
 * </code>
 */
class BundleUtilWrapper
{
public:
  BundleUtilWrapper() : _lastError() {}
  ~BundleUtilWrapper() = default;

  BundleUtilWrapper(const BundleUtilWrapper&) = delete;
  BundleUtilWrapper& operator=(const BundleUtilWrapper&) = delete;

  BundleUtilWrapper(const BundleUtilWrapper&& other) : _lastError(std::move(other._lastError)) {}
  BundleUtilWrapper& operator=(const BundleUtilWrapper&& other) = delete;
	
  std::string GetLibraryPath(void* symbol) 
  {
    std::string result(BundleUtils::GetLibraryPath(symbol));
    _lastError = GetLastError();
    return result; 
  }

  void* GetSymbol(const std::string& bundleName, const std::string& libLocation, const char* symbol) 
  {
    void* result(BundleUtils::GetSymbol(bundleName, libLocation, symbol));
    _lastError = GetLastError();
    return result; 
  }

  std::string LastError() { return _lastError; }

private:
  std::string GetLastError()
  {
#if defined(US_PLATFORM_POSIX)
    const char* str = dlerror();
    return std::string(((str == nullptr) ? "" : str));
#else
    return us::GetLastErrorStr();
#endif
  }

  std::string _lastError;
};

namespace {
#ifdef US_BUILD_SHARED_LIBS
  const bool sharedLibMode = true;
#else
  const bool sharedLibMode = false;
#endif
}

namespace BundleUtils
{

#ifdef __GNUC__

std::string GetLibraryPath_impl(void* symbol)
{
  Dl_info info;
  if (dladdr(symbol, &info))
  {
    return info.dli_fname;
  }
  
  return std::string();
}

void* GetSymbol_impl(const std::string& bundleName, const std::string& libLocation, const char* symbol)
{
  // Clear the last error message
  dlerror();

  void* selfHandle = nullptr;
  if (!sharedLibMode || bundleName == "main")
  {
    // Get the handle of the executable
    selfHandle = dlopen(0, RTLD_LAZY);
  }
  else
  {
    selfHandle = dlopen(libLocation.c_str(), RTLD_LAZY);
  }

  if (selfHandle)
  {
    void* addr = dlsym(selfHandle, symbol);

    dlclose(selfHandle);
    return addr;
  }

  return nullptr;
}

#elif _WIN32

std::string GetLibraryPath_impl(void *symbol)
{
  HMODULE handle = nullptr;
  BOOL handleError = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                       static_cast<LPCTSTR>(symbol), &handle);
  if (!handleError) return std::string();

  char bundlePath[512];
  if (GetModuleFileName(handle, bundlePath, 512))
  {
    return bundlePath;
  }

  return std::string();
}

void* GetSymbol_impl(const std::string& bundleName, const std::string& libLocation, const char* symbol)
{
  HMODULE handle = nullptr;
  if (!sharedLibMode || bundleName == "main")
  {
    handle = GetModuleHandle(nullptr);
  }
  else
  {
    handle = GetModuleHandle(libLocation.c_str());
  }

  if (!handle) return nullptr;

  return (void*)GetProcAddress(handle, symbol);
}

#else

std::string GetLibraryPath_impl(void*)
{
  return "";
}

void* GetSymbol_impl(const std::string&, const std::string&, const char*)
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
  return GetSymbol_impl(bundleName, libLocation, symbol);
}

} // namespace BundleUtils

} // namespace us
