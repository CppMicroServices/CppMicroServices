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

#include "usCoreBundleContext_p.h"
#include "usGetBundleContext.h"
#include "usLog.h"
#include "usBundleInfo.h"
#include "usBundle.h"
#include "usBundleContext.h"

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
  #include <unistd.h>   // getcwd
#else
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #include <crtdbg.h>
  #include "dirent_win32.h"
#endif

namespace {
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

#ifdef US_PLATFORM_POSIX

const char DIR_SEP = '/';

#elif defined(US_PLATFORM_WINDOWS)

const char DIR_SEP = '\\';

#endif
}

namespace us {

//-------------------------------------------------------------------
// Bundle name and location parsing
//-------------------------------------------------------------------

void ExtractBundleNameAndLocation(const std::string& locationName, std::string& outLocation, std::string& outName)
{
  if (locationName.empty())
  {
    return;
  }
  size_t pos = locationName.find_last_of('|');
  if (pos != std::string::npos)
  {
    outName = locationName.substr(pos+1);
    outLocation = locationName.substr(0, pos);
  }
  else
  {
    outLocation = locationName;
  }
}

bool IsSharedLibrary(const std::string& location)
{ // Testing for file extension isn't the most robust way to test
    // for file type.
    return (location.find(library_suffix()) != std::string::npos);
}

//-------------------------------------------------------------------
// Bundle auto-loading
//-------------------------------------------------------------------


std::vector<std::string> AutoInstallBundlesFromPath(const std::string& absoluteBasePath, const std::string& subDir)
{
  std::vector<std::string> installedBundles;

  std::string loadPath = subDir.empty() ? absoluteBasePath : absoluteBasePath + DIR_SEP + subDir;
  
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
      US_DEBUG << "Auto-installing bundle " << libPath;
      try
      {
        std::shared_ptr<Bundle> installedBundle = GetBundleContext()->InstallBundle(libPath);
        if (!installedBundle)
        {
          US_WARN << "Auto-installing of bundle " << libPath << " failed.";
        }
        else
        {
          installedBundles.push_back(installedBundle->GetName());
        }
      }
      catch (const std::runtime_error& exp)
      {
        US_WARN << "Auto-installing of bundle " << libPath << " failed - " << exp.what();
      }
    }
    closedir(dir);
  }
  return installedBundles;
}
  
  std::vector<std::string> GetBundleNamesFromLibrary(const std::string& libPath)
  {
    std::vector<std::string> bundleNames;
    mz_zip_archive zipArchive;
    memset(&zipArchive, 0, sizeof(mz_zip_archive));
    if(MZ_FALSE != mz_zip_reader_init_file(&zipArchive, libPath.c_str(), 0))
    {
    
    // the usResourceCompiler will place resources into sub directories,
    // one for each bundle, named after the bundle's name. The bundle's
    // manifest is stored in a file called manifest.json in the root of
    // its sub-directory (analogous to OSGi's META-INF/MANIFEST.MF file).
    // We use this convention to glean the bundle name and use that to
    // install the bundle.
    mz_uint numFiles = mz_zip_reader_get_num_files(&zipArchive);
    for (mz_uint fileIndex = 0; fileIndex < numFiles; ++fileIndex)
    {
      char fileName[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
      mz_zip_reader_get_filename(&zipArchive, fileIndex, fileName, MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE);
      std::string file(fileName);
      std::string::size_type pos = file.find("/manifest.json");
      if(std::string::npos != pos)
      {
        bundleNames.push_back(file.substr(0, pos));
      }
    }
    }
    mz_zip_reader_end(&zipArchive);
    return bundleNames;
  }
  
  

std::vector<std::string> AutoInstallBundles(const BundleInfo& bundleInfo, CoreBundleContext* coreCtx)
{
  std::vector<std::string> installedBundles;

  if (bundleInfo.autoLoadDir.empty())
  {
    return installedBundles;
  }

  BundleSettings::PathList autoLoadPaths = coreCtx->settings.GetAutoLoadPaths();

  std::size_t indexOfLastSeparator = bundleInfo.location.find_last_of(DIR_SEP);
  std::string bundleBasePath = bundleInfo.location.substr(0, indexOfLastSeparator);

  for (BundleSettings::PathList::iterator i = autoLoadPaths.begin();
       i != autoLoadPaths.end(); ++i)
  {
    if (*i == BundleSettings::CURRENT_BUNDLE_PATH())
    {
      // Load all bundles from a directory located relative to this bundles location
      // and named after this bundles library name.
      *i = bundleBasePath;
    }
  }

  // We could have introduced a duplicate above, so remove it.
  std::sort(autoLoadPaths.begin(), autoLoadPaths.end());
  autoLoadPaths.erase(std::unique(autoLoadPaths.begin(), autoLoadPaths.end()), autoLoadPaths.end());
  for (BundleSettings::PathList::iterator i = autoLoadPaths.begin();
       i != autoLoadPaths.end(); ++i)
  {
    if (i->empty()) continue;
    std::vector<std::string> paths = AutoInstallBundlesFromPath(*i, bundleInfo.autoLoadDir);
    installedBundles.insert(installedBundles.end(), paths.begin(), paths.end());
  }
  return installedBundles;
}

}

//-------------------------------------------------------------------
// Generic utility functions
//-------------------------------------------------------------------

namespace us {

std::string GetCurrentWorkingDirectory()
{
#ifdef US_PLATFORM_WINDOWS
  DWORD bufSize = ::GetCurrentDirectoryA(0, NULL);
  if (bufSize == 0) bufSize = 1;
  std::shared_ptr<char> buf(make_shared_array<char>(bufSize));
  if (::GetCurrentDirectoryA(bufSize, buf.get()) != 0)
  {
    return std::string(buf.get());
  }
#else
  std::size_t bufSize = PATH_MAX;
  for(;; bufSize *= 2)
  {
    std::shared_ptr<char> buf(make_shared_array<char>(bufSize));
    errno = 0;
    if (getcwd(buf.get(), bufSize) != 0 && errno != ERANGE)
    {
      return std::string(buf.get());
    }
  }
#endif
  return std::string();
}

//-------------------------------------------------------------------
// Error handling
//-------------------------------------------------------------------

std::string GetLastErrorStr()
{
#ifdef US_PLATFORM_POSIX
  return std::string(strerror(errno));
#else
  // Retrieve the system error message for the last-error code
  LPVOID lpMsgBuf;
  DWORD dw = GetLastError();

  DWORD rc = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0, NULL );

  // If FormatMessage fails using FORMAT_MESSAGE_ALLOCATE_BUFFER
  // it means that the size of the error message exceeds an internal
  // buffer limit (128 kb according to MSDN) and lpMsgBuf will be 
  // uninitialized.
  // Inform the caller that the error message couldn't be retrieved.
  if (rc == 0)
  {
    return std::string("Failed to retrieve error message.");
  }

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

US_Core_EXPORT ::std::string detail::GetDemangledName(const ::std::type_info& typeInfo)
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

}
