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

#include "cppmicroservices/util/FileSystem.h"
#include "cppmicroservices/util/Error.h"
#include "cppmicroservices/util/String.h"
#include <cppmicroservices/GlobalConfig.h>

#include <stdexcept>
#include <string>
#include <vector>
#include <locale> 
#include <codecvt>

#ifdef US_PLATFORM_POSIX
#  include <dirent.h>
#  include <dlfcn.h>
#  include <cerrno>
#  include <cstring>
#  include <errno.h>
#  include <string.h>
#  include <sys/time.h>
#  include <unistd.h> // getcwd
#  include <cstdlib>  // getenv
#  define US_STAT struct stat
#  define us_stat stat
#  define us_mkdir mkdir
#  define us_rmdir rmdir
#  define us_unlink unlink
#else
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <Shlwapi.h>
#  include <crtdbg.h>
#  include <direct.h>
#  include <io.h>
#  include <stdint.h>
#  include <windows.h>

#  ifdef __MINGW32__
#    include <dirent.h>
#  else
#      include "dirent_win32.h"
#  endif
#  define US_STAT struct _stat
#  define us_stat _stat
#  define us_mkdir _mkdir
#  define us_rmdir _rmdir
#  define us_unlink _unlink
#endif

#ifdef US_PLATFORM_APPLE
#  include <mach-o/dyld.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sys/stat.h> // mkdir, _S_IREAD, etc.

namespace cppmicroservices {

namespace util {

#ifdef US_PLATFORM_WINDOWS
bool not_found_win32_error(int errval)
{
  return errval == ERROR_FILE_NOT_FOUND || errval == ERROR_PATH_NOT_FOUND ||
         errval == ERROR_INVALID_NAME // "//foo"
         ||
         errval == ERROR_INVALID_DRIVE // USB card reader with no card inserted
         || errval == ERROR_NOT_READY  // CD/DVD drive with no disc inserted
         || errval == ERROR_INVALID_PARAMETER // ":sys:stat.h"
         || errval == ERROR_BAD_PATHNAME      // "//nosuch" on Win64
         || errval == ERROR_BAD_NETPATH;      // "//nosuch" on Win32
}
#endif

#ifdef US_PLATFORM_WINDOWS
const char DIR_SEP = DIR_SEP_WIN32;
#else
const char DIR_SEP = DIR_SEP_POSIX;
#endif

bool not_found_c_error(int errval)
{
  return errval == ENOENT || errval == ENOTDIR;
}

std::vector<std::string> SplitString(const std::string& str,
                                     const std::string& delim)
{
  std::vector<std::string> token;
  std::size_t b = str.find_first_not_of(delim);
  std::size_t e = str.find_first_of(delim, b);
  while (e > b) {
    token.emplace_back(str.substr(b, e - b));
    b = str.find_first_not_of(delim, e);
    e = str.find_first_of(delim, b);
  }
  return token;
}

#ifndef MAXPATHLEN
#  define MAXPATHLEN 1024
#endif

std::string GetExecutablePath()
{
  uint32_t bufsize = MAXPATHLEN;
#ifdef US_PLATFORM_WINDOWS
  std::vector<wchar_t> wbuf(bufsize + 1, '\0');
  if (GetModuleFileNameW(nullptr, wbuf.data(), bufsize) == 0 ||
      GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
    throw std::runtime_error("GetModuleFileName failed" +
                             GetLastWin32ErrorStr());
  }
  return ToUTF8String(wbuf.data());
#elif defined(US_PLATFORM_APPLE)
  std::vector<char> buf(bufsize + 1, '\0');
  int status = _NSGetExecutablePath(buf.data(), &bufsize);
  if (status == -1) {
    buf.assign(bufsize + 1, '\0');
    status = _NSGetExecutablePath(buf.data(), &bufsize);
  }
  if (status != 0) {
    throw std::runtime_error("_NSGetExecutablePath() failed");
  }
  // the returned path may not be an absolute path
  return buf.data();
#elif defined(US_PLATFORM_LINUX)
  std::vector<char> buf(bufsize + 1, '\0');
  ssize_t len = ::readlink("/proc/self/exe", buf.data(), bufsize);
  if (len == -1 || len == static_cast<ssize_t>(bufsize)) {
    throw std::runtime_error("Could not read /proc/self/exe into buffer");
  }
  return buf.data();
#else
  // 'dlsym' does not work with symbol name 'main'
  throw std::runtime_error("GetExecutablePath failed");
  return "";
#endif
}

std::string InitCurrentWorkingDirectory()
{
#ifdef US_PLATFORM_WINDOWS
  DWORD bufSize = ::GetCurrentDirectoryW(0, NULL);
  if (bufSize == 0)
    bufSize = 1;
  std::vector<wchar_t> buf(bufSize, L'\0');
  if (::GetCurrentDirectoryW(bufSize, buf.data()) != 0) {
    return util::ToUTF8String(buf.data());
  }
#else
  std::size_t bufSize = PATH_MAX;
  for (;; bufSize *= 2) {
    std::vector<char> buf(bufSize, '\0');
    errno = 0;
    if (getcwd(buf.data(), bufSize) != nullptr && errno != ERANGE) {
      return std::string(buf.data());
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
  errno = 0;
  if (us_stat(path.c_str(), &s)) {
    if (not_found_c_error(errno))
      return false;
    else
      throw std::invalid_argument(GetLastCErrorStr());
  }
#else
  std::wstring wpath(ToWString(path));
  DWORD attr(::GetFileAttributesW(wpath.c_str()));
  if (attr == INVALID_FILE_ATTRIBUTES) {
    if (not_found_win32_error(::GetLastError()))
      return false;
    else
      throw std::invalid_argument(GetLastWin32ErrorStr());
  }
#endif
  return true;
}

bool IsDirectory(const std::string& path)
{
  US_STAT s;
  errno = 0;
  if (us_stat(path.c_str(), &s)) {
    if (not_found_c_error(errno))
      return false;
    else
      throw std::invalid_argument(GetLastCErrorStr());
  }
  return S_ISDIR(s.st_mode);
}

bool IsFile(const std::string& path)
{
  US_STAT s;
  errno = 0;
  if (us_stat(path.c_str(), &s)) {
    if (not_found_c_error(errno))
      return false;
    else
      throw std::invalid_argument(GetLastCErrorStr());
  }
  return S_ISREG(s.st_mode);
}

bool IsRelative(const std::string& path)
{
#ifdef US_PLATFORM_WINDOWS
  if (path.size() > MAX_PATH)
    return false;
  std::wstring wpath(ToWString(path));
  return (TRUE == ::PathIsRelativeW(wpath.c_str())) ? true : false;
#else
  return path.empty() || path[0] != DIR_SEP;
#endif
}

std::string GetAbsolute(const std::string& path, const std::string& base)
{
  if (IsRelative(path))
    return base + DIR_SEP + path;
  return path;
}

void MakePath(const std::string& path)
{
  std::string subPath;
  auto dirs = SplitString(path, std::string() + DIR_SEP_WIN32 + DIR_SEP_POSIX);
  if (dirs.empty())
    return;

  auto iter = dirs.begin();
#ifdef US_PLATFORM_POSIX
  // Start with the root '/' directory
  subPath = DIR_SEP;
#else
  // Start with the drive letter`
  subPath = *iter + DIR_SEP;
  ++iter;
#endif
  for (; iter != dirs.end(); ++iter) {
    subPath += *iter;
    errno = 0;
#ifdef US_PLATFORM_WINDOWS
    if (us_mkdir(subPath.c_str()))
#else
    if (us_mkdir(subPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
#endif
    {
      if (errno != EEXIST)
        throw std::invalid_argument(GetLastCErrorStr());
    }
    subPath += DIR_SEP;
  }
}

void RemoveDirectoryRecursive(const std::string& path)
{
  int res = -1;
  errno = 0;
  DIR* dir = opendir(path.c_str());
  if (dir != nullptr) {
    res = 0;

    struct dirent* ent = nullptr;
    while (!res && (ent = readdir(dir)) != nullptr) {
      // Skip the names "." and ".." as we don't want to recurse on them.
      if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
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
      } else {
        res = us_unlink(child.c_str());
      }
    }
    int old_err = errno;
    errno = 0;
    closedir(dir); // error ignored
    if (old_err) {
      errno = old_err;
    }
  }

  if (!res) {
    errno = 0;
    res = us_rmdir(path.c_str());
  }

  if (res)
  {
	  std::string message = "Unable to delete folder '" + path + "' ";
	  message += "(" + GetLastCErrorStr() + ")";
	  throw std::invalid_argument(message);
  }
}

std::string GetTempDirectory()
{
#if defined(US_PLATFORM_WINDOWS)
  std::wstring temp_dir;
  wchar_t wcharFullPath[MAX_PATH];
  wchar_t wcharPath[MAX_PATH];
  GetTempPathW(MAX_PATH, wcharPath);
  if (wcharPath && GetLongPathNameW(wcharPath, wcharFullPath, MAX_PATH)) {
    temp_dir = wcharFullPath;
  }

  using convert_type = std::codecvt_utf8<wchar_t>;
  std::wstring_convert<convert_type, wchar_t> converter;
  return converter.to_bytes(temp_dir);
#else
  char* tempdir = std::getenv("TMPDIR");
  return std::string(((tempdir == nullptr) ? "/tmp" : tempdir));
#endif
}

static const char validLetters[] =
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

// A cross-platform version of the mkstemps function
static int mkstemps_compat(char* tmpl, int suffixlen)
{
  static unsigned long long value = 0;
  int savedErrno = errno;

// Lower bound on the number of temporary files to attempt to generate.
#define ATTEMPTS_MIN (62 * 62 * 62)

/* The number of times to attempt to generate a temporary file.  To
conform to POSIX, this must be no smaller than TMP_MAX.  */
#if ATTEMPTS_MIN < TMP_MAX
  const unsigned int attempts = TMP_MAX;
#else
  const unsigned int attempts = ATTEMPTS_MIN;
#endif

  const std::size_t len = strlen(tmpl);
  if ((len - suffixlen) < 6 ||
      strncmp(&tmpl[len - 6 - suffixlen], "XXXXXX", 6)) {
    errno = EINVAL;
    return -1;
  }

  /* This is where the Xs start.  */
  char* XXXXXX = &tmpl[len - 6 - suffixlen];

/* Get some more or less random data.  */
#ifdef US_PLATFORM_WINDOWS
  {
    SYSTEMTIME stNow;
    FILETIME ftNow;

    // get system time
    GetSystemTime(&stNow);
    stNow.wMilliseconds = 500;
    if (!SystemTimeToFileTime(&stNow, &ftNow)) {
      errno = -1;
      return -1;
    }
    unsigned long long randomTimeBits =
      ((static_cast<unsigned long long>(ftNow.dwHighDateTime) << 32) |
       static_cast<unsigned long long>(ftNow.dwLowDateTime));
    value =
      randomTimeBits ^ static_cast<unsigned long long>(GetCurrentThreadId());
  }
#else
  {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    unsigned long long randomTimeBits =
      ((static_cast<unsigned long long>(tv.tv_usec) << 32) |
       static_cast<unsigned long long>(tv.tv_sec));
    value = randomTimeBits ^ static_cast<unsigned long long>(getpid());
  }
#endif

  for (unsigned int count = 0; count < attempts; value += 7777, ++count) {
    unsigned long long v = value;

    /* Fill in the random bits.  */
    XXXXXX[0] = validLetters[v % 62];
    v /= 62;
    XXXXXX[1] = validLetters[v % 62];
    v /= 62;
    XXXXXX[2] = validLetters[v % 62];
    v /= 62;
    XXXXXX[3] = validLetters[v % 62];
    v /= 62;
    XXXXXX[4] = validLetters[v % 62];
    v /= 62;
    XXXXXX[5] = validLetters[v % 62];

#ifdef US_PLATFORM_WINDOWS
    int fd = _open(tmpl, O_RDWR | O_CREAT | O_EXCL, _S_IREAD | _S_IWRITE);
#else
    int fd = open(tmpl, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
#endif
    if (fd >= 0) {
      errno = savedErrno;
      return fd;
    } else if (errno != EEXIST) {
      return -1;
    }
  }

  /* We got out of the loop because we ran out of combinations to try.  */
  errno = EEXIST;
  return -1;
}

// A cross-platform version of the POSIX mkdtemp function
char* mkdtemps_compat(char* tmpl, int suffixlen)
{
  static unsigned long long value = 0;
  int savedErrno = errno;

// Lower bound on the number of temporary dirs to attempt to generate.
#define ATTEMPTS_MIN (62 * 62 * 62)

/* The number of times to attempt to generate a temporary dir.  To
conform to POSIX, this must be no smaller than TMP_MAX.  */
#if ATTEMPTS_MIN < TMP_MAX
  const unsigned int attempts = TMP_MAX;
#else
  const unsigned int attempts = ATTEMPTS_MIN;
#endif

  const std::size_t len = strlen(tmpl);
  if ((len - suffixlen) < 6 ||
      strncmp(&tmpl[len - 6 - suffixlen], "XXXXXX", 6)) {
    errno = EINVAL;
    return nullptr;
  }

  /* This is where the Xs start.  */
  char* XXXXXX = &tmpl[len - 6 - suffixlen];

/* Get some more or less random data.  */
#ifdef US_PLATFORM_WINDOWS
  {
    SYSTEMTIME stNow;
    FILETIME ftNow;

    // get system time
    GetSystemTime(&stNow);
    stNow.wMilliseconds = 500;
    if (!SystemTimeToFileTime(&stNow, &ftNow)) {
      errno = -1;
      return nullptr;
    }
    unsigned long long randomTimeBits =
      ((static_cast<unsigned long long>(ftNow.dwHighDateTime) << 32) |
       static_cast<unsigned long long>(ftNow.dwLowDateTime));
    value =
      randomTimeBits ^ static_cast<unsigned long long>(GetCurrentThreadId());
  }
#else
  {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    unsigned long long randomTimeBits =
      ((static_cast<unsigned long long>(tv.tv_usec) << 32) |
       static_cast<unsigned long long>(tv.tv_sec));
    value = randomTimeBits ^ static_cast<unsigned long long>(getpid());
  }
#endif

  unsigned int count = 0;
  for (; count < attempts; value += 7777, ++count) {
    unsigned long long v = value;

    /* Fill in the random bits.  */
    XXXXXX[0] = validLetters[v % 62];
    v /= 62;
    XXXXXX[1] = validLetters[v % 62];
    v /= 62;
    XXXXXX[2] = validLetters[v % 62];
    v /= 62;
    XXXXXX[3] = validLetters[v % 62];
    v /= 62;
    XXXXXX[4] = validLetters[v % 62];
    v /= 62;
    XXXXXX[5] = validLetters[v % 62];

#ifdef US_PLATFORM_WINDOWS
    int r = _mkdir(tmpl); //, _S_IREAD | _S_IWRITE | _S_IEXEC);
#else
    int r = mkdir(tmpl, S_IRUSR | S_IWUSR | S_IXUSR);
#endif
    if (r >= 0) {
      errno = savedErrno;
      return tmpl;
    } else if (errno != EEXIST) {
      return nullptr;
    }
  }

  /* We got out of the loop because we ran out of combinations to try.  */
  errno = EEXIST;
  return nullptr;
}

File::File()
  : FileDescr(-1)
  , Path()
{}

File::File(int fd, const std::string& path)
  : FileDescr(fd)
  , Path(path)
{}

File::File(File&& o)
  : FileDescr(o.FileDescr)
  , Path(std::move(o.Path))
{
  o.FileDescr = -1;
}

File& File::operator=(File&& o)
{
  std::swap(FileDescr, o.FileDescr);
  std::swap(Path, o.Path);
  return *this;
}

File::~File()
{
  if (FileDescr >= 0)
    close(FileDescr);
}

std::string MakeUniqueTempDirectory()
{
  std::string tmpStr = util::GetTempDirectory();
  if (!tmpStr.empty() && *--tmpStr.end() != util::DIR_SEP) {
    tmpStr += util::DIR_SEP;
  }
  tmpStr += "usdir-XXXXXX";
  std::vector<char> tmpChars(tmpStr.c_str(),
                             tmpStr.c_str() + tmpStr.length() + 1);

  errno = 0;
  if (!mkdtemps_compat(tmpChars.data(), 0))
    throw std::runtime_error(util::GetLastCErrorStr());

  return tmpChars.data();
}

File MakeUniqueTempFile(const std::string& base)
{
  const auto tmpStr = base + util::DIR_SEP + "usfile-XXXXXX";
  std::vector<char> tmpChars(tmpStr.c_str(),
                             tmpStr.c_str() + tmpStr.length() + 1);

  errno = 0;
  int fd = mkstemps_compat(tmpChars.data(), 0);
  if (fd < 0)
    throw std::runtime_error(util::GetLastCErrorStr());

  return File(fd, tmpChars.data());
}

} // namespace util
} // namespace cppmicroservices
