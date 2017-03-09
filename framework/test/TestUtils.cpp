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

#include "TestUtils.h"

#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/GlobalConfig.h"

#include "TestingConfig.h"
#include "TestingMacros.h"

#if defined(US_PLATFORM_WINDOWS)
#include <Shlobj.h> // SHGetKnownFolderPath
#endif

namespace cppmicroservices {

#if defined(US_PLATFORM_APPLE)

double HighPrecisionTimer::timeConvert = 0.0;

HighPrecisionTimer::HighPrecisionTimer()
    : startTime(0)
{
    if (timeConvert == 0)
    {
        mach_timebase_info_data_t timeBase;
        mach_timebase_info(&timeBase);
        timeConvert = static_cast<double>(timeBase.numer) / static_cast<double>(timeBase.denom) / 1000.0;
    }
}

void HighPrecisionTimer::Start()
{
    startTime = mach_absolute_time();
}

long long HighPrecisionTimer::ElapsedMilli()
{
    uint64_t current = mach_absolute_time();
    return static_cast<double>(current - startTime) * timeConvert / 1000.0;
}

long long HighPrecisionTimer::ElapsedMicro()
{
    uint64_t current = mach_absolute_time();
    return static_cast<double>(current - startTime) * timeConvert;
}

#elif defined(US_PLATFORM_POSIX)

HighPrecisionTimer::HighPrecisionTimer()
{
    startTime.tv_nsec = 0;
    startTime.tv_sec = 0;
}

void HighPrecisionTimer::Start()
{
    clock_gettime(CLOCK_MONOTONIC, &startTime);
}

long long HighPrecisionTimer::ElapsedMilli()
{
    timespec current;
    clock_gettime(CLOCK_MONOTONIC, &current);
    return (static_cast<long long>(current.tv_sec) * 1000 + current.tv_nsec / 1000 / 1000) -
        (static_cast<long long>(startTime.tv_sec) * 1000 + startTime.tv_nsec / 1000 / 1000);
}

long long HighPrecisionTimer::ElapsedMicro()
{
    timespec current;
    clock_gettime(CLOCK_MONOTONIC, &current);
    return (static_cast<long long>(current.tv_sec) * 1000 * 1000 + current.tv_nsec / 1000) -
        (static_cast<long long>(startTime.tv_sec) * 1000 * 1000 + startTime.tv_nsec / 1000);
}

#elif defined(US_PLATFORM_WINDOWS)

HighPrecisionTimer::HighPrecisionTimer()
{
    if (!QueryPerformanceFrequency(&timerFrequency))
        throw std::runtime_error("QueryPerformanceFrequency() failed");
}

void HighPrecisionTimer::Start()
{
    //DWORD_PTR oldmask = SetThreadAffinityMask(GetCurrentThread(), 0);
    QueryPerformanceCounter(&startTime);
    //SetThreadAffinityMask(GetCurrentThread(), oldmask);
}

long long HighPrecisionTimer::ElapsedMilli()
{
    LARGE_INTEGER current;
    QueryPerformanceCounter(&current);
    return (current.QuadPart - startTime.QuadPart) / (timerFrequency.QuadPart / 1000);
}

long long HighPrecisionTimer::ElapsedMicro()
{
    LARGE_INTEGER current;
    QueryPerformanceCounter(&current);
    return (current.QuadPart - startTime.QuadPart) / (timerFrequency.QuadPart / (1000 * 1000));
}

#endif


namespace testing {

Bundle InstallLib(BundleContext frameworkCtx, const std::string& libName)
{
    std::vector<Bundle> bundles;
    try
    {
#if defined (US_BUILD_SHARED_LIBS)
        bundles = frameworkCtx.InstallBundles(LIB_PATH + DIR_SEP + US_LIB_PREFIX + libName + US_LIB_EXT);
#else
        bundles = frameworkCtx.GetBundles();
#endif
        US_TEST_CONDITION_REQUIRED(!bundles.empty(), "Test installation of library " << libName)
    }
    catch (const std::exception& e)
    {
        US_TEST_FAILED_MSG(<< "Install bundle exception: " << e.what())
    }
    for (auto b : bundles)
    {
      if (b.GetSymbolicName() == libName) return b;
    }
    return {};
}

std::string GetTempDirectory()
{
#if defined (US_PLATFORM_WINDOWS)
  std::wstring temp_dir;
  PWSTR buffer = nullptr;
  HRESULT rc = SHGetKnownFolderPath(FOLDERID_ProgramData, KF_FLAG_DEFAULT, nullptr, &buffer);

  if (SUCCEEDED(rc)) temp_dir = buffer;
  if (buffer) CoTaskMemFree(static_cast<void*>(buffer));
  return std::string(temp_dir.cbegin(), temp_dir.cend());
#else
    char* tempdir = getenv("TMPDIR");
  return std::string(((tempdir == nullptr)?"":tempdir));
#endif
}

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
  for (;; bufSize *= 2)
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

#ifdef US_PLATFORM_WINDOWS
#define rmdir _rmdir
#define chdir _chdir
#endif

void ChangeDirectory(const std::string& destdir)
{
  int ret = chdir(destdir.c_str());
  if (ret != 0)
  {
    std::string msg("Unable to change directory to ");
    msg += destdir + ". Does this directory exist?";
    throw std::runtime_error(msg.c_str());
  }
}

void MakeDirectory(const std::string& dirname)
{
#ifdef US_PLATFORM_WINDOWS
  int ret = _mkdir(dirname.c_str());
#else
  // rws permissions for owner, group. rs permissions for others.
  int ret = mkdir(dirname.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
  if (ret != 0)
  {
    std::string msg("Unable to make directory:");
    msg += dirname + ". Does this directory already exist?";
    throw std::runtime_error(msg.c_str());
  }
}

void RemoveDirectory(const std::string& dirname)
{
  int ret = rmdir(dirname.c_str());
  if (ret != 0)
  {
    std::string msg("Unable to remove directory:");
    msg += dirname + ". Is this directory empty?";
    throw std::runtime_error(msg.c_str());
  }
}

void CheckFileAndRemove(std::string f)
{
  std::ifstream fobj(f.c_str());
  if (!fobj.good())
  {
    fobj.close();
    return;
  }
  fobj.close();

  if (remove(f.c_str()) != 0)
  {
    throw std::runtime_error("Could not remove file " + f);

  }
}

bool DirectoryExists(const std::string& destdir)
{
  // stat() is cross-platform
  struct stat dirinfo;
  if (stat(destdir.c_str(), &dirinfo) != 0)
  {
    return false;
  }
  else if (dirinfo.st_mode & S_IFDIR)
  {
    return true;
  }
  return false;
}

Bundle GetBundle(const std::string& bsn, BundleContext context)
{
  if (!context) context = cppmicroservices::GetBundleContext();
  for (auto b : context.GetBundles())
  {
    if (b.GetSymbolicName() == bsn) return b;
  }
  return {};
}

}

}
