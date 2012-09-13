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

#include "usTestUtilSharedLibrary.h"

#include <stdexcept>

#include <usTestingConfig.h>
#include <usUtils_p.h>
#include <usModuleRegistry.h>
#include <usModuleActivator.h>


#if defined(US_PLATFORM_POSIX)
  #include <dlfcn.h>
#elif defined(US_PLATFORM_WINDOWS)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <strsafe.h>
#else
  #error Unsupported platform
#endif


US_BEGIN_NAMESPACE

SharedLibraryHandle::SharedLibraryHandle() : m_Handle(0) {}

SharedLibraryHandle::SharedLibraryHandle(const std::string& name)
  : m_Name(name), m_Handle(0)
{}

SharedLibraryHandle::~SharedLibraryHandle()
{}

void SharedLibraryHandle::Load()
{
  Load(m_Name);
}

void SharedLibraryHandle::Load(const std::string& name)
{
#ifdef US_BUILD_SHARED_LIBS
  if (m_Handle) throw std::logic_error(std::string("Library already loaded: ") + name);
  std::string libPath = GetAbsolutePath(name);
#ifdef US_PLATFORM_POSIX
  m_Handle = dlopen(libPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  if (!m_Handle)
  {
    const char* err = dlerror();
    throw std::runtime_error(err ? std::string(err) : libPath);
  }
#else
  m_Handle = LoadLibrary(libPath.c_str());
  if (!m_Handle)
  {
    std::string errMsg = "Loading ";
    errMsg.append(libPath).append("failed with error: ").append(GetLastErrorStr());

    throw std::runtime_error(errMsg);
  }
#endif

#endif

  m_Name = name;
}

void SharedLibraryHandle::Unload()
{
#ifdef US_BUILD_SHARED_LIBS
  if (m_Handle)
  {
#ifdef US_PLATFORM_POSIX
    dlclose(m_Handle);
#else
    FreeLibrary((HMODULE)m_Handle);
#endif
    m_Handle = 0;
  }
#endif
}

std::string SharedLibraryHandle::GetAbsolutePath(const std::string& name)
{
  return GetLibraryPath() + "/" + Prefix() + name + Suffix();
}

std::string SharedLibraryHandle::GetAbsolutePath()
{
  return GetLibraryPath() + "/" + Prefix() + m_Name + Suffix();
}

std::string SharedLibraryHandle::GetLibraryPath()
{
#ifdef US_PLATFORM_WINDOWS
  return std::string(US_RUNTIME_OUTPUT_DIRECTORY);
#else
  return std::string(US_LIBRARY_OUTPUT_DIRECTORY);
#endif
}

std::string SharedLibraryHandle::Suffix()
{
#ifdef US_PLATFORM_WINDOWS
  return ".dll";
#elif defined(US_PLATFORM_APPLE)
  return ".dylib";
#else
  return ".so";
#endif
}

std::string SharedLibraryHandle::Prefix()
{
#if defined(US_PLATFORM_POSIX)
  return "lib";
#else
  return "";
#endif
}

US_END_NAMESPACE
