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

#include "usTestUtils.h"
#include "usTestingConfig.h"
#include "usTestingMacros.h"

#include <usGlobalConfig.h>

namespace us {

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

Module* InstallTestBundle(ModuleContext* frameworkCtx, const std::string& bundleName)
{
    Module* module = nullptr;
    try
    {
#if defined (US_BUILD_SHARED_LIBS)
        module = frameworkCtx->InstallBundle(LIB_PATH + DIR_SEP + LIB_PREFIX + bundleName + LIB_EXT + "/" + bundleName);
#else
        module = frameworkCtx->InstallBundle(BIN_PATH + DIR_SEP + "usCoreTestDriver" + EXE_EXT + "/" + bundleName);
#endif
        US_TEST_CONDITION_REQUIRED(module != NULL, "Test installation of module " << bundleName)
    }
    catch (const std::exception& e)
    {
        US_TEST_FAILED_MSG(<< "Install bundle exception: " << e.what())
    }
    return module;
}

}
