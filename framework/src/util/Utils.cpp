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

#include "Utils.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/detail/Log.h"

#include "BundleResourceContainer.h"
#include "CoreBundleContext.h"

#include <algorithm>
#include <cstdio>
#include <cctype>
#include <string>
#include <typeinfo>

#ifdef US_PLATFORM_POSIX
  #include <dlfcn.h>
  #include <dirent.h>
  #include <errno.h>
  #include <string.h>
  #include <unistd.h>   // getcwd

  #define US_STAT struct stat
  #define us_stat stat
  #define us_mkdir mkdir
  #define us_rmdir rmdir
  #define us_unlink unlink
#else
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #include <Shlwapi.h>
  #include <crtdbg.h>
  #include <direct.h>
#ifdef __MINGW32__
  #include <dirent.h>
#else
  #include "dirent_win32.h"
#endif
#define US_STAT struct __stat64
int us_stat(const char *path,
			US_STAT *buffer)
{
	std::unique_ptr<wchar_t[]> wpath(cppmicroservices::UTF8ToWchar(path));
	return _wstat64(wpath.get(), buffer);
}

int us_mkdir(const char* path)
{
	std::unique_ptr<wchar_t[]> wpath(cppmicroservices::UTF8ToWchar(path));
	return _wmkdir(wpath.get());
}

int us_rmdir(const char* path)
{
	std::unique_ptr<wchar_t[]> wpath(cppmicroservices::UTF8ToWchar(path));
	return _wrmdir(wpath.get());
}

int us_unlink(const char* path)
{
	std::unique_ptr<wchar_t[]> wpath(cppmicroservices::UTF8ToWchar(path));
	return _wunlink(wpath.get());
}

#endif

#ifdef US_HAVE_CXXABI_H
#include <cxxabi.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>

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

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

}


namespace cppmicroservices {
	std::unique_ptr<wchar_t[]> UTF8ToWchar(const char* inStr)
	{
		if (!inStr)
		{
			return nullptr;
		}
		int wchars_count = MultiByteToWideChar(CP_UTF8, 0, inStr, -1, NULL, 0);
		std::unique_ptr<wchar_t[]> wstr(new wchar_t[wchars_count]);
		MultiByteToWideChar(CP_UTF8, 0, inStr, -1, wstr.get(), wchars_count);
		return wstr;
	}
//-------------------------------------------------------------------
// File system functions
//-------------------------------------------------------------------

namespace fs {

bool not_found_error(int errval)
{
#ifdef US_PLATFORM_WINDOWS
  return errval == ERROR_FILE_NOT_FOUND
      || errval == ERROR_PATH_NOT_FOUND
      || errval == ERROR_INVALID_NAME       // "//foo"
      || errval == ERROR_INVALID_DRIVE      // USB card reader with no card inserted
      || errval == ERROR_NOT_READY          // CD/DVD drive with no disc inserted
      || errval == ERROR_INVALID_PARAMETER  // ":sys:stat.h"
      || errval == ERROR_BAD_PATHNAME       // "//nosuch" on Win64
      || errval == ERROR_BAD_NETPATH;       // "//nosuch" on Win32

#else
  return errval == ENOENT || errval == ENOTDIR;
#endif
}

std::vector<std::string> SplitString(const std::string& str, const std::string& delim)
{
  std::vector<std::string> token;
  std::size_t b = str.find_first_not_of(delim);
  std::size_t e = str.find_first_of(delim, b);
  while (e > b)
  {
    token.emplace_back(str.substr(b, e - b));
    b = str.find_first_not_of(delim, e);
    e = str.find_first_of(delim, b);
  }
  return token;
}

std::string InitCurrentWorkingDirectory()
{
#ifdef US_PLATFORM_WINDOWS
  DWORD bufSize = ::GetCurrentDirectoryW(0, NULL);
  if (bufSize == 0) bufSize = 1;
  std::shared_ptr<wchar_t> buf(make_shared_array<wchar_t>(bufSize));
  if (::GetCurrentDirectoryW(bufSize, buf.get()) != 0)
  {
    return utf8_encode(buf.get());
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

static const std::string s_CurrWorkingDir = InitCurrentWorkingDirectory();

std::string GetCurrentWorkingDirectory()
{
  return s_CurrWorkingDir;
}

bool Exists(const std::string& path)
{
#ifdef US_PLATFORM_POSIX
  US_STAT s;
  if (us_stat(path.c_str(), &s))
  {
    if (not_found_error(errno)) return false;
    else throw std::invalid_argument(GetLastErrorStr());
  }
#else
  std::unique_ptr<wchar_t[]> wpath(UTF8ToWchar(path.c_str()));
  DWORD attr(::GetFileAttributesW(wpath.get()));
  if (attr == INVALID_FILE_ATTRIBUTES)
  {
    if (not_found_error(::GetLastError())) return false;
    else throw std::invalid_argument(GetLastErrorStr());
  }
#endif
  return true;
}

bool IsDirectory(const std::string& path)
{
  US_STAT s;
  if (us_stat(path.c_str(), &s))
  {
    if (not_found_error(errno)) return false;
    else throw std::invalid_argument(GetLastErrorStr());
  }
  return S_ISDIR(s.st_mode);
}

bool IsFile(const std::string& path)
{
  US_STAT s;
  if (us_stat(path.c_str(), &s))
  {
    if (not_found_error(errno)) return false;
    else throw std::invalid_argument(GetLastErrorStr());
  }
  return S_ISREG(s.st_mode);

}

bool IsRelative(const std::string& path)
{
#ifdef US_PLATFORM_WINDOWS
  if (path.size() > MAX_PATH) return false;
  std::unique_ptr<wchar_t[]> wpath(UTF8ToWchar(path.c_str()));
  return (TRUE == ::PathIsRelativeW(wpath.get())) ? true:false;
#else
  return path.empty() || path[0] != DIR_SEP;
#endif
}

std::string GetAbsolute(const std::string& path, const std::string& base)
{
  if (IsRelative(path)) return base + DIR_SEP + path;
  return path;
}

void MakePath(const std::string& path)
{
  std::string subPath;
  auto dirs = SplitString(path, std::string() + DIR_SEP_WIN32 + DIR_SEP_POSIX);
  if (dirs.empty()) return;

  auto iter = dirs.begin();
#ifdef US_PLATFORM_POSIX
  // Start with the root '/' directory
  subPath = DIR_SEP;
#else
  // Start with the drive letter`
  subPath = *iter + DIR_SEP;
  ++iter;
#endif
  for (; iter != dirs.end(); ++iter)
  {
    subPath += *iter;
#ifdef US_PLATFORM_WINDOWS
    if (us_mkdir(subPath.c_str()))
    {
      if (GetLastErrorNo() != ERROR_ALREADY_EXISTS) throw std::invalid_argument(GetLastErrorStr());
    }
#else
    if (us_mkdir(subPath.c_str(), S_IRWXU))
    {
      if (GetLastErrorNo() != EEXIST) throw std::invalid_argument(GetLastErrorStr());
    }
#endif
    subPath += DIR_SEP;
  }
}

void RemoveDirectoryRecursive(const std::string& path)
{
  int res = -1;
  DIR* dir = opendir(path.c_str());
  if (dir != nullptr)
  {
    res = 0;

    struct dirent *ent = nullptr;
    while (!res && (ent = readdir(dir)) != nullptr)
    {
      // Skip the names "." and ".." as we don't want to recurse on them.
      if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
      {
        continue;
      }

      std::string child = path + DIR_SEP + ent->d_name;
      if
#ifdef _DIRENT_HAVE_D_TYPE
          (ent->d_type == DT_DIR)
#else
          (IsDirectory(child))
#endif
      {
        RemoveDirectoryRecursive(child);
      }
      else
      {
        res = us_unlink(child.c_str());
      }
    }
    closedir(dir);
  }

  if (!res) res = us_rmdir(path.c_str());

  if (res) throw std::invalid_argument(GetLastErrorStr());
}

} // namespace fs


//-------------------------------------------------------------------
// Bundle name and location parsing
//-------------------------------------------------------------------

bool IsSharedLibrary(const std::string& location)
{ // Testing for file extension isn't the most robust way to test
    // for file type.
    return (location.find(library_suffix()) != std::string::npos);
}

bool IsBundleFile(const std::string& location)
{
  // We require a zip file with at least one top-level directory
  // containing a manifest.json file for a file to be a valid bundle.
  try
  {
    // If this location is a bundle, a top level directory will
    // contain a manifest.json file at its root. There is no need
    // to recursively search nested directories.
    BundleResourceContainer resContainer(location);
    auto topLevelDirs = resContainer.GetTopLevelDirs();
    return std::any_of(topLevelDirs.begin(), topLevelDirs.end(), [&resContainer](const std::string& dir) -> bool {

      std::vector<std::string> names;
      std::vector<uint32_t> indices;

      resContainer.GetChildren(dir + "/", true, names, indices);
      return std::any_of(names.begin(), names.end(), [](const std::string& resourceName) -> bool {
        return resourceName == std::string("manifest.json");
      });
    });
  }
  catch (...)
  {
    return false;
  }
}


//-------------------------------------------------------------------
// Framework storage
//-------------------------------------------------------------------

const std::string FWDIR_DEFAULT = "fwdir";

std::string GetFrameworkDir(CoreBundleContext* ctx)
{
  auto it = ctx->frameworkProperties.find(Constants::FRAMEWORK_STORAGE);
  if (it == ctx->frameworkProperties.end() || it->second.Type() != typeid(std::string))
  {
    return FWDIR_DEFAULT;
  }
  return any_cast<std::string>(it->second);
}

std::string GetFileStorage(CoreBundleContext* ctx, const std::string& name, bool create)
{
  // See if we have a storage directory
  const std::string fwdir = GetFrameworkDir(ctx);
  if (fwdir.empty())
  {
    return fwdir;
  }
  const std::string dir = fs::GetAbsolute(fwdir, ctx->workingDir) + DIR_SEP + name;
  if (!dir.empty())
  {
    if (fs::Exists(dir))
    {
      if (!fs::IsDirectory(dir))
      {
        throw std::runtime_error("Not a directory: " + dir);
      }
    }
    else
    {
      if (create)
      {
        try { fs::MakePath(dir); }
        catch (const std::exception& e)
        {
          throw std::runtime_error("Cannot create directory: " + dir + " (" + e.what() + ")");
        }
      }
    }
  }
  return dir;
}

//-------------------------------------------------------------------
// Error handling
//-------------------------------------------------------------------

int GetLastErrorNo()
{
#ifdef US_PLATFORM_POSIX
  return errno;
#else
  return ::GetLastError();
#endif
}

std::string GetLastErrorStr()
{
#ifdef US_PLATFORM_POSIX
  char* errorString = strerror(errno);
  return std::string(((errorString == nullptr)?"":errorString));
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
                reinterpret_cast<LPTSTR>(&lpMsgBuf),
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

  std::string errMsg(reinterpret_cast<LPCTSTR>(lpMsgBuf));

  LocalFree(lpMsgBuf);

  return errMsg;
#endif
}

void TerminateForDebug(const std::exception_ptr ex)
{
#if defined(_MSC_VER) && !defined(NDEBUG) && defined(_DEBUG) && defined(_CRT_ERROR)
    std::string message = GetLastExceptionStr();

    // get the current report mode
    int reportMode = _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);
    _CrtSetReportMode(_CRT_ERROR, reportMode);
    int ret = _CrtDbgReport(_CRT_ERROR, __FILE__, __LINE__, CppMicroServices_VERSION_STR, message.c_str());
    if (ret == 0  && reportMode & _CRTDBG_MODE_WNDW)
      return; // ignore
    else if (ret == 1)
      _CrtDbgBreak();
#else
    (void)ex;
#endif

#ifdef US_PLATFORM_POSIX
  abort(); // trap; generates core dump
#else
  exit(1); // goodbye cruel world
#endif
}

namespace detail
{
std::string GetDemangledName(const std::type_info& typeInfo)
{
  std::string result;
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
  if (demangled != nullptr)
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

US_MSVC_PUSH_DISABLE_WARNING(4715) // 'function' : not all control paths return a value
std::string GetExceptionStr(const std::exception_ptr& exc)
{
  if (!exc)
  {
    return std::string();
  }

  try
  {
    std::rethrow_exception(exc);
  }
  catch (const std::exception& e)
  {
    return e.what();
  }
  catch (...)
  {
    return "unknown";
  }
}
US_MSVC_POP_WARNING

std::string GetLastExceptionStr()
{
  return GetExceptionStr(std::current_exception());
}

} // namespace cppmicroservices
