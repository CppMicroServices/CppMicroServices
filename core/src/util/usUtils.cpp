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

#include "usUtils_p.h"

#include "usGetModuleContext.h"
#include "usLog_p.h"
#include "usModuleInfo.h"
#include "usModule.h"
#include "usModuleContext.h"
#include "usModuleSettings.h"
#include "usModuleRegistry.h"

#include "miniz.h"

#include <string>
#include <cstdio>
#include <cctype>
#include <algorithm>
#include <typeinfo>

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
  #include "dirent_win32.h"
#endif

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

#elif defined(US_PLATFORM_WINDOWS)

const char DIR_SEP = '\\';

#endif
}

US_BEGIN_NAMESPACE

//-------------------------------------------------------------------
// Bundle name and location parsing
//-------------------------------------------------------------------

std::string GetBundleNameFromLocation(const std::string& location)
{
    return location.substr(location.find_last_of('/') + 1);
}

std::string GetBundleLocation(const std::string& location)
{
    return location.substr(0, location.find_last_of('/'));
}

//-------------------------------------------------------------------
// Module auto-loading
//-------------------------------------------------------------------


std::vector<std::string> AutoLoadModulesFromPath(const std::string& absoluteBasePath, const std::string& subDir, CoreModuleContext* coreCtx)
{
  std::vector<std::string> loadedModules;

  std::string loadPath = absoluteBasePath + DIR_SEP + subDir;

  DIR* dir = opendir(loadPath.c_str());
#ifdef CMAKE_INTDIR
  // Try intermediate output directories
  if (dir == NULL)
  {
    std::size_t indexOfLastSeparator = absoluteBasePath.find_last_of(DIR_SEP);
    if (indexOfLastSeparator != std::string::npos)
    {
      std::string intermediateDir = absoluteBasePath.substr(indexOfLastSeparator+1);
      bool equalSubDir = intermediateDir.size() == std::strlen(CMAKE_INTDIR);
      for (std::size_t i = 0; equalSubDir && i < intermediateDir.size(); ++i)
      {
        if (std::tolower(intermediateDir[i]) != std::tolower(CMAKE_INTDIR[i]))
        {
          equalSubDir = false;
        }
      }
      if (equalSubDir)
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
      US_DEBUG << "Auto-installing module " << libPath;

      mz_zip_archive zipArchive;
      memset(&zipArchive, 0, sizeof(mz_zip_archive));
      if(MZ_FALSE == mz_zip_reader_init_file(&zipArchive, libPath.c_str(), 0)) continue;

      // the usResourceCompiler will place resources into sub directories,
      // one for each module, named after the module's name. The module's
      // manifest is stored in a file called manifest.json in the root of
      // its sub-directory (analogous to OSGi's META-INF/MANIFEST.MF file).
      // We use this convention to glean the module name and use that to
      // install the module.
      mz_uint numFiles = mz_zip_reader_get_num_files(&zipArchive);
      for (mz_uint fileIndex = 0; fileIndex < numFiles; ++fileIndex)
      {
        char fileName[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
        mz_zip_reader_get_filename(&zipArchive, fileIndex, fileName, MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE);
        std::string file(fileName);
        std::string::size_type pos = file.find("/manifest.json");
        if(std::string::npos != pos)
        {
          std::string moduleName(file.substr(0, pos));
          std::string location(libPath + "/" + moduleName);

          // location will be in the form:
          //  <path to module plugin>\<module-name>.<lib-extension>/<module-name>
          Module* installedModule = GetModuleContext(coreCtx)->InstallBundle(location);

          if (!installedModule)
          {
            US_WARN << "Auto-installing of module " << libPath << " failed.";
          }
          else
          {
            loadedModules.push_back(installedModule->GetName());
          }
        }
      }

      mz_zip_reader_end(&zipArchive);
    }
    closedir(dir);
  }
  return loadedModules;
}

std::vector<std::string> AutoLoadModules(const ModuleInfo& moduleInfo, CoreModuleContext* coreCtx)
{
  std::vector<std::string> loadedModules;

  if (moduleInfo.autoLoadDir.empty())
  {
    return loadedModules;
  }

  ModuleSettings::PathList autoLoadPaths = coreCtx->settings.GetAutoLoadPaths();

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
    std::vector<std::string> paths = AutoLoadModulesFromPath(*i, moduleInfo.autoLoadDir, coreCtx);
    loadedModules.insert(loadedModules.end(), paths.begin(), paths.end());
  }
  return loadedModules;
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

#ifdef US_HAVE_CXXABI_H
#include <cxxabi.h>
#endif

US_Core_EXPORT ::std::string GetDemangledName(const ::std::type_info& typeInfo)
{
  ::std::string result;
#ifdef US_HAVE_CXXABI_H
  int status = 0;
  char* demangled = abi::__cxa_demangle(typeInfo.name(), 0, 0, &status);
  if (demangled && status == 0)
  {
    result = demangled;
    free(demangled);
  }
#elif defined(US_PLATFORM_WINDOWS)
  const char* demangled = typeInfo.name();
  if (demangled != NULL)
  {
    result = demangled;
    // remove "struct" qualifiers
    std::size_t pos = 0;
    while (pos != std::string::npos)
    {
      if ((pos = result.find("struct ", pos)) != std::string::npos)
      {
        result = result.substr(0, pos) + result.substr(pos + 7);
        pos += 8;
      }
    }
    // remove "class" qualifiers
    pos = 0;
    while (pos != std::string::npos)
    {
      if ((pos = result.find("class ", pos)) != std::string::npos)
      {
        result = result.substr(0, pos) + result.substr(pos + 6);
        pos += 7;
      }
    }
  }
#else
  (void)typeInfo;
#endif
  return result;
}

US_END_NAMESPACE
