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


#include "usModuleUtils_p.h"
#include <usUtils_p.h>

US_BEGIN_NAMESPACE

namespace {
#ifdef US_BUILD_SHARED_LIBS
  const bool sharedLibMode = true;
#else
  const bool sharedLibMode = false;
#endif
}

#ifdef __GNUC__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>

std::string GetLibraryPath_impl(const std::string& /*libName*/, void* symbol)
{
  Dl_info info;
  if (dladdr(symbol, &info))
  {
    return info.dli_fname;
  }
  else
  {
    US_DEBUG << "GetLibraryPath_impl() dladdr() failed: " << dlerror();
  }
  return "";
}

void* GetSymbol_impl(const std::string& libName, const char* symbol)
{
  // Clear the last error message
  dlerror();

  void* selfHandle = 0;
  if (libName.empty() || !sharedLibMode)
  {
    // Get the handle of the executable
    selfHandle = dlopen(0, RTLD_LAZY);
  }
  else
  {
    selfHandle = dlopen(libName.c_str(), RTLD_LAZY);
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
  return 0;
}

#elif _WIN32

#include <windows.h>

std::string GetLibraryPath_impl(const std::string& libName, void *symbol)
{
  HMODULE handle = 0;
  if (libName.empty() || !sharedLibMode)
  {
    // get the handle for the executable
    handle = GetModuleHandle(NULL);
  }
  else
  {
    handle = GetModuleHandle(libName.c_str());
  }
  if (!handle)
  {
    // Test
    US_DEBUG << "GetLibraryPath_impl():GetModuleHandle() " << GetLastErrorStr();
    return "";
  }

  char modulePath[512];
  if (GetModuleFileName(handle, modulePath, 512))
  {
    return modulePath;
  }

  US_DEBUG << "GetLibraryPath_impl():GetModuleFileName() " << GetLastErrorStr();
  return "";
}

void* GetSymbol_impl(const std::string& libName, const char* symbol)
{
  HMODULE handle = NULL;
  if (libName.empty() || !sharedLibMode)
  {
    handle = GetModuleHandle(NULL);
  }
  else
  {
    handle = GetModuleHandle(libName.c_str());
  }

  if (!handle)
  {
    US_DEBUG << "GetSymbol_impl():GetModuleHandle() " << GetLastErrorStr();
    return 0;
  }

  void* addr = (void*)GetProcAddress(handle, symbol);
  if (!addr)
  {
    US_DEBUG << "GetSymbol_impl():GetProcAddress(handle," << symbol << ") " << GetLastErrorStr();
  }
  return addr;
}
#else
std::string GetLibraryPath_impl(const std::string& libName, void* symbol)
{
  return "";
}

void* GetSymbol_impl(const std::string& libName, const char* symbol)
{
  return 0;
}
#endif

std::string ModuleUtils::GetLibraryPath(const std::string& libName, void* symbol)
{
  return GetLibraryPath_impl(libName, symbol);
}

void* ModuleUtils::GetSymbol(const std::string& libName, const char* symbol)
{
  return GetSymbol_impl(libName, symbol);
}

US_END_NAMESPACE
