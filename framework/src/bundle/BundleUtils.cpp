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

#include "BundleUtils.h"

#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/detail/Log.h"

#include "BundleContextPrivate.h"
#include "BundlePrivate.h"
#include "CoreBundleContext.h"
#include "Utils.h"

#ifdef __GNUC__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#include <sys/param.h>
#endif
#include <limits.h>
#include <unistd.h>
#elif _WIN32
#include <windows.h>

#define RTLD_LAZY 0 // unused

const char* dlerror(void)
{
  static std::string errStr;
  errStr = cppmicroservices::GetLastErrorStr();
  return errStr.c_str();
}

void* dlopen(const char * path, int mode)
{
  (void)mode; // ignored
  return reinterpret_cast<void*>(path == nullptr ? GetModuleHandle(nullptr) : LoadLibrary(path));
}

void* dlsym(void *handle, const char *symbol)
{
  return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(handle), symbol));
}

#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

namespace cppmicroservices {

// Private util function to return system bundle's log sink
std::shared_ptr<detail::LogSink> GetFrameworkLogSink()
{
  // The following is a hack, we need a cleaner solution in the future
  return GetPrivate(GetBundleContext())->bundle->coreCtx->sink;
}

namespace BundleUtils
{
  
void* GetExecutableHandle()
{
  return dlopen(0, RTLD_LAZY);;
}

std::string GetExecutablePath()
{
  std::string execPath;
  uint32_t bufsize = MAXPATHLEN;
  std::unique_ptr<char[]> buf(new char[bufsize]);
  
#if _WIN32
  if (GetModuleFileName(nullptr, buf.get(), bufsize) == 0 || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    DIAG_LOG(*GetFrameworkLogSink()) << "GetModuleFileName failed" << GetLastErrorStr() << "\n";
    buf[0] = '\0';
  }
#elif defined(__APPLE__)
  int status = _NSGetExecutablePath(buf.get(), &bufsize);
  if (status == -1)
  {
    buf.reset(new char[bufsize]);
    status = _NSGetExecutablePath(buf.get(), &bufsize);
  }
  if (status != 0)
  {
     DIAG_LOG(*GetFrameworkLogSink()) << "_NSGetExecutablePath() failed\n";
  }
  // the returned path may not be an absolute path
#elif defined(__linux__)
  ssize_t len = ::readlink("/proc/self/exe", buf.get(), bufsize);
  if (len == -1 || len == bufsize)
  {
    len = 0;
  }
  buf[len] = '\0';
#else
  // 'dlsym' does not work with symbol name 'main'
  DIAG_LOG(*GetFrameworkLogSink()) << "GetExecutablePath failed\n";
#endif
  return buf.get();
}

void* GetSymbol(void* libHandle, const char* symbol)
{
  void* addr = libHandle ? dlsym(libHandle, symbol) : nullptr;
  if (!addr)
  {
    const char* dlerrorMsg = dlerror();
    DIAG_LOG(*GetFrameworkLogSink()) << "GetSymbol() failed to find (" << symbol << ") with error : "<< (dlerrorMsg ? dlerrorMsg : "unknown") << "\n";
  }
  return addr;
}
  
} // namespace BundleUtils

} // namespace cppmicroservices
