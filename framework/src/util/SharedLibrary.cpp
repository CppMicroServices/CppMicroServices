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

#include "cppmicroservices/SharedLibrary.h"

#include "cppmicroservices/BundleActivator.h"

#include "cppmicroservices/util/FileSystem.h"

#if defined(US_PLATFORM_POSIX)
#    include <cerrno>
#    include <dlfcn.h>
#elif defined(US_PLATFORM_WINDOWS)
#    include "cppmicroservices/util/Error.h"
#    include "cppmicroservices/util/String.h"
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
// clang-format off
  // Do not re-order include directives, it would break MinGW builds.
#  include <windows.h>
#  include <strsafe.h>
// clang-format on
#else
#    error Unsupported platform
#endif

#include <stdexcept>

namespace cppmicroservices
{

    class SharedLibraryPrivate
    {
      public:
        SharedLibraryPrivate() : m_Suffix(US_LIB_EXT), m_Prefix(US_LIB_PREFIX) {}

        void* m_Handle { nullptr };

        std::string m_Name;
        std::string m_Path;
        std::string m_FilePath;
        std::string m_Suffix;
        std::string m_Prefix;
    };

    SharedLibrary::SharedLibrary() : d(new SharedLibraryPrivate) {}

    SharedLibrary::SharedLibrary(SharedLibrary const&) = default;

    SharedLibrary::SharedLibrary(std::string const& libPath, std::string const& name) : d(new SharedLibraryPrivate)
    {
        d->m_Name = name;
        d->m_Path = libPath;
    }

    SharedLibrary::SharedLibrary(std::string const& absoluteFilePath) : d(new SharedLibraryPrivate)
    {
        d->m_FilePath = absoluteFilePath;
        SetFilePath(absoluteFilePath);
    }

    SharedLibrary::~SharedLibrary() = default;

    SharedLibrary& SharedLibrary::operator=(SharedLibrary const& other) = default;

    void
    SharedLibrary::Load(int flags)
    {
        if (d->m_Handle)
            throw std::logic_error(std::string("Library already loaded: ") + GetFilePath());
        std::string libPath = GetFilePath();
#ifdef US_PLATFORM_POSIX
        d->m_Handle = dlopen(libPath.c_str(), flags);
        if (!d->m_Handle)
        {
            std::error_code err_code(errno, std::generic_category());
            std::string err_msg = "Error loading " + libPath + ".";
            char const* err = dlerror();
            if (err)
            {
                err_msg += " " + std::string(err);
            }

            d->m_Handle = nullptr;

            // Bundle of origin information is not available here. It will be
            // BundlePrivate::Start0() will catch this system_error and create
            // a SharedLibraryException.
            throw std::system_error(err_code, err_msg);
        }
#else
        US_UNUSED(flags);
        std::wstring wpath(cppmicroservices::util::ToWString(libPath));
        d->m_Handle = LoadLibraryW(wpath.c_str());

        if (!d->m_Handle)
        {
            std::error_code err_code(GetLastError(), std::generic_category());
            std::string errMsg = "Loading ";
            errMsg.append(libPath).append("failed with error: ").append(util::GetLastWin32ErrorStr());

            d->m_Handle = nullptr;

            // Bundle of origin information is not available here. Use try/catch
            // around SharedLibrary::Load(), and throw a SharedLibraryException
            // inside the catch statement, with the available bundle of origin.
            throw std::system_error(err_code, errMsg);
        }
#endif
    }

    void
    SharedLibrary::Load()
    {
#ifdef US_PLATFORM_POSIX
        Load(RTLD_LAZY | RTLD_LOCAL);
#else
        Load(0);
#endif
    }

    void
    SharedLibrary::Unload()
    {
        if (d->m_Handle)
        {
#ifdef US_PLATFORM_POSIX
            if (dlclose(d->m_Handle))
            {
                std::string err_msg = "Error unloading " + GetLibraryPath() + ".";
                char const* err = dlerror();
                if (err)
                {
                    err_msg += " " + std::string(err);
                }

                d->m_Handle = nullptr;
                throw std::runtime_error(err_msg);
            }
#else
            if (!FreeLibrary(reinterpret_cast<HMODULE>(d->m_Handle)))
            {
                std::string errMsg = "Unloading ";
                errMsg.append(GetLibraryPath()).append("failed with error: ").append(util::GetLastWin32ErrorStr());

                d->m_Handle = nullptr;
                throw std::runtime_error(errMsg);
            }
#endif

            d->m_Handle = nullptr;
        }
    }

    void
    SharedLibrary::SetName(std::string const& name)
    {
        if (IsLoaded() || !d->m_FilePath.empty())
            return;

        d->m_Name = name;
    }

    std::string
    SharedLibrary::GetName() const
    {
        return d->m_Name;
    }

    std::string
    SharedLibrary::GetFilePath(std::string const& name) const
    {
        if (!d->m_FilePath.empty())
            return d->m_FilePath;
        return GetLibraryPath() + util::DIR_SEP + GetPrefix() + name + GetSuffix();
    }

    void
    SharedLibrary::SetFilePath(std::string const& absoluteFilePath)
    {
        if (IsLoaded())
            return;

        d->m_FilePath = absoluteFilePath;

        std::string name = d->m_FilePath;
        std::size_t pos = d->m_FilePath.find_last_of(util::DIR_SEP);
        if (pos != std::string::npos)
        {
            d->m_Path = d->m_FilePath.substr(0, pos);
            name = d->m_FilePath.substr(pos + 1);
        }
        else
        {
            d->m_Path.clear();
        }

        if (name.size() >= d->m_Prefix.size() && name.compare(0, d->m_Prefix.size(), d->m_Prefix) == 0)
        {
            name = name.substr(d->m_Prefix.size());
        }
        if (name.size() >= d->m_Suffix.size()
            && name.compare(name.size() - d->m_Suffix.size(), d->m_Suffix.size(), d->m_Suffix) == 0)
        {
            name = name.substr(0, name.size() - d->m_Suffix.size());
        }
        d->m_Name = name;
    }

    std::string
    SharedLibrary::GetFilePath() const
    {
        return GetFilePath(d->m_Name);
    }

    void
    SharedLibrary::SetLibraryPath(std::string const& path)
    {
        if (IsLoaded() || !d->m_FilePath.empty())
            return;

        d->m_Path = path;
    }

    std::string
    SharedLibrary::GetLibraryPath() const
    {
        return d->m_Path;
    }

    void
    SharedLibrary::SetSuffix(std::string const& suffix)
    {
        if (IsLoaded() || !d->m_FilePath.empty())
            return;

        d->m_Suffix = suffix;
    }

    std::string
    SharedLibrary::GetSuffix() const
    {
        return d->m_Suffix;
    }

    void
    SharedLibrary::SetPrefix(std::string const& prefix)
    {
        if (IsLoaded() || !d->m_FilePath.empty())
            return;

        d->m_Prefix = prefix;
    }

    std::string
    SharedLibrary::GetPrefix() const
    {
        return d->m_Prefix;
    }

    void*
    SharedLibrary::GetHandle() const
    {
        return d->m_Handle;
    }

    bool
    SharedLibrary::IsLoaded() const
    {
        return d->m_Handle != nullptr;
    }
} // namespace cppmicroservices
