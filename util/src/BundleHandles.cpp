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

#include "cppmicroservices/util/BundleHandles.h"
#include <cppmicroservices/GlobalConfig.h>

#ifdef US_PLATFORM_WINDOWS

#    include "cppmicroservices/util/Error.h"
#    include "cppmicroservices/util/String.h"
#    include <windows.h>

#    define RTLD_LAZY 0 // unused

std::string
dlerror(void)
{
    return cppmicroservices::util::GetLastWin32ErrorStr();
}

void*
dlopen(char const* path, int)
{
    auto loadLibrary = [](std::string const& path) -> HANDLE
    {
        std::wstring wpath(cppmicroservices::util::ToWString(path));
        return LoadLibraryW(wpath.c_str());
    };
    return reinterpret_cast<void*>(path == nullptr ? GetModuleHandleW(nullptr) : loadLibrary(path));
}

void*
dlsym(void* handle, char const* symbol)
{
    return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(handle), symbol));
}

#elif defined(__GNUC__)

#    ifndef _GNU_SOURCE
#        define _GNU_SOURCE
#    endif

#    include <dlfcn.h>

#    if defined(__APPLE__)
#        include <mach-o/dyld.h>
#        include <sys/param.h>
#    endif

#    include <unistd.h>

#endif

namespace cppmicroservices
{

    namespace util
    {

        void*
        GetExecutableHandle()
        {
            return dlopen(nullptr, RTLD_LAZY);
            ;
        }

        void*
        GetSymbol(void* libHandle, char const* symbol, std::string& errmsg)
        {
            void* addr = libHandle ? dlsym(libHandle, symbol) : nullptr;
            if (!addr)
            {
                const std::string dlerrorMsg = dlerror();
                errmsg += "GetSymbol() failed to find (" + std::string{symbol} +
                    ") with error : " + (!dlerrorMsg.empty() ? dlerrorMsg : "unknown");
            }
            return addr;
        }

    } // namespace util

} // namespace cppmicroservices