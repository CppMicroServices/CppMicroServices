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

#ifndef USTESTUTILS_H
#define USTESTUTILS_H

#include "usBundle.h"
#include "usBundleContext.h"

#include <string>
#include <memory>

#ifdef US_PLATFORM_APPLE
#include <mach/mach_time.h>
#include <unistd.h>
#elif defined(US_PLATFORM_POSIX)
#include <limits.h>
#include <time.h>
#include <unistd.h>
#ifndef _POSIX_MONOTONIC_CLOCK
#error Monotonic clock support missing on this POSIX platform
#endif
#elif defined(US_PLATFORM_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef VC_EXTRA_LEAN
#define VC_EXTRA_LEAN
#endif
#include <windows.h>
#else
#error High precision timer support not available on this platform
#endif

namespace us {

class HighPrecisionTimer
{

public:

    HighPrecisionTimer();

    void Start();

    long long ElapsedMilli();

    long long ElapsedMicro();

private:

#ifdef US_PLATFORM_APPLE
    static double timeConvert;
    uint64_t startTime;
#elif defined(US_PLATFORM_POSIX)
    timespec startTime;
#elif defined(US_PLATFORM_WINDOWS)
    LARGE_INTEGER timerFrequency;
    LARGE_INTEGER startTime;
#endif
};

// Helper function to install bundles, given a framework's bundle context and the name of the bundle.
// Assumes that test bundles are within the same directory during unit testing.
// Currently limited to only installing bundles with the same physical filename
// and logical bundle name (e.g. TestBundleA.dll/TestBundleA).
std::shared_ptr<Bundle> InstallTestBundle(BundleContext* frameworkCtx, const std::string& bundleName);

// Copied from usUtils_p.h/usUtils.cpp
// Place in a different namespace to avoid duplicate symbol errors.
namespace testing {

// A convenient way to construct a shared_ptr holding an array
template<typename T> std::shared_ptr<T> make_shared_array(std::size_t size)
{
  return std::shared_ptr<T>(new T[size], std::default_delete<T[]>());
}

std::string GetCurrentWorkingDirectory();
}

}

#endif  // USTESTUTILS_H
