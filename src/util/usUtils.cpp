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

#include "usUtils_p.h"

#include "usModuleInfo.h"
#include "usModuleSettings.h"

#include <cstdio>

#ifdef US_PLATFORM_POSIX
  #include <errno.h>
  #include <string.h>
  #include <dlfcn.h>
  #include <dirent.h>
#else
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #include <crtdbg.h>
  #include "dirent_win32_p.h"
#endif

//-------------------------------------------------------------------
// Module auto-loading
//-------------------------------------------------------------------

namespace {

#if !defined(US_PLATFORM_LINUX)
std::string library_suffix()
{
#ifdef US_PLATFORM_WINDOWS
  return ".dll";
#elif defined(US_PLATFORM_APPLE)
  return ".dylib";
#else
  return ".so";
#endif
}
#endif

#ifdef US_PLATFORM_POSIX

const char DIR_SEP = '/';

bool load_impl(const std::string& modulePath)
{
  void* handle = dlopen(modulePath.c_str(), RTLD_NOW | RTLD_LOCAL);
  return (handle != NULL);
}

#elif defined(US_PLATFORM_WINDOWS)

const char DIR_SEP = '\\';

bool load_impl(const std::string& modulePath)
{
  void* handle = LoadLibrary(modulePath.c_str());
  return (handle != NULL);
}

#else

  #ifdef US_ENABLE_AUTOLOADING_SUPPORT
    #error "Missing load_impl implementation for this platform."
  #else
bool load_impl(const std::string&) { return false; }
  #endif

#endif

}

US_BEGIN_NAMESPACE

void AutoLoadModulesFromPath(const std::string& absoluteBasePath, const std::string& subDir)
{
  std::string loadPath = absoluteBasePath + DIR_SEP + subDir;

  DIR* dir = opendir(loadPath.c_str());
#ifdef CMAKE_INTDIR
  // Try intermediate output directories
  if (dir == NULL)
  {
    std::size_t indexOfLastSeparator = absoluteBasePath.find_last_of(DIR_SEP);
    if (indexOfLastSeparator != -1)
    {
      if (absoluteBasePath.substr(indexOfLastSeparator+1) == CMAKE_INTDIR)
      {
        loadPath = absoluteBasePath.substr(0, indexOfLastSeparator+1) + subDir + DIR_SEP + CMAKE_INTDIR;
        dir = opendir(loadPath.c_str());
      }
    }
  }
#endif

  if (dir != NULL)
  {
    struct dirent *ent = NULL;
    while ((ent = readdir(dir)) != NULL)
    {
      bool loadFile = true;
#ifdef _DIRENT_HAVE_D_TYPE
      if (ent->d_type != DT_UNKNOWN && ent->d_type != DT_REG)
      {
        loadFile = false;
      }
#endif

      std::string entryFileName(ent->d_name);

      // On Linux, library file names can have version numbers appended. On other platforms, we
      // check the file ending. This could be refined for Linux in the future.
#if !defined(US_PLATFORM_LINUX)
      if (entryFileName.rfind(library_suffix()) != (entryFileName.size() - library_suffix().size()))
      {
        loadFile = false;
      }
#endif
      if (!loadFile) continue;

      std::string libPath = loadPath;
      if (!libPath.empty() && libPath.find_last_of(DIR_SEP) != libPath.size() -1)
      {
        libPath += DIR_SEP;
      }
      libPath += entryFileName;
      US_INFO << "Auto-loading module " << libPath;

      if (!load_impl(libPath))
      {
        US_WARN << "Auto-loading of module " << libPath << " failed: " << GetLastErrorStr();
      }
    }
    closedir(dir);
  }
}

void AutoLoadModules(const ModuleInfo& moduleInfo)
{
  if (moduleInfo.autoLoadDir.empty())
  {
    return;
  }

  ModuleSettings::PathList autoLoadPaths = ModuleSettings::GetAutoLoadPaths();

  std::size_t indexOfLastSeparator = moduleInfo.location.find_last_of(DIR_SEP);
  std::string moduleBasePath = moduleInfo.location.substr(0, indexOfLastSeparator);

  for (ModuleSettings::PathList::iterator i = autoLoadPaths.begin();
       i != autoLoadPaths.end(); ++i)
  {
    if (*i == ModuleSettings::CURRENT_MODULE_PATH())
    {
      // Load all modules from a directory located relative to this modules location
      // and named after this modules library name.
      *i = moduleBasePath;
    }
  }

  // We could have introduced a duplicate above, so remove it.
  std::sort(autoLoadPaths.begin(), autoLoadPaths.end());
  autoLoadPaths.erase(std::unique(autoLoadPaths.begin(), autoLoadPaths.end()), autoLoadPaths.end());
  for (ModuleSettings::PathList::iterator i = autoLoadPaths.begin();
       i != autoLoadPaths.end(); ++i)
  {
    if (i->empty()) continue;
    AutoLoadModulesFromPath(*i, moduleInfo.autoLoadDir);
  }
}

US_END_NAMESPACE

//-------------------------------------------------------------------
// Error handling
//-------------------------------------------------------------------

US_BEGIN_NAMESPACE

std::string GetLastErrorStr()
{
#ifdef US_PLATFORM_POSIX
  return std::string(strerror(errno));
#else
  // Retrieve the system error message for the last-error code
  LPVOID lpMsgBuf;
  DWORD dw = GetLastError();

  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    dw,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR) &lpMsgBuf,
    0, NULL );

  std::string errMsg((LPCTSTR)lpMsgBuf);

  LocalFree(lpMsgBuf);

  return errMsg;
#endif
}

static MsgHandler handler = 0;

MsgHandler installMsgHandler(MsgHandler h)
{
  MsgHandler old = handler;
  handler = h;
  return old;
}

void message_output(MsgType msgType, const char *buf)
{
  if (handler)
  {
    (*handler)(msgType, buf);
  }
  else
  {
    fprintf(stderr, "%s\n", buf);
    fflush(stderr);
  }

  if (msgType == ErrorMsg)
  {
  #if defined(_MSC_VER) && !defined(NDEBUG) && defined(_DEBUG) && defined(_CRT_ERROR)
    // get the current report mode
    int reportMode = _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);
    _CrtSetReportMode(_CRT_ERROR, reportMode);
    int ret = _CrtDbgReport(_CRT_ERROR, __FILE__, __LINE__, CppMicroServices_VERSION_STR, buf);
    if (ret == 0  && reportMode & _CRTDBG_MODE_WNDW)
      return; // ignore
    else if (ret == 1)
      _CrtDbgBreak();
  #endif

  #ifdef US_PLATFORM_POSIX
    abort(); // trap; generates core dump
  #else
    exit(1); // goodbye cruel world
  #endif
  }
}

US_END_NAMESPACE
