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

#ifndef ANDROIDCOMPAT_H
#define ANDROIDCOMPAT_H

/**
 * Compatibility layer for Android
 *
 * This file and AndroidCompat.cpp exist to allow CppMicroServices
 * to use std::to_string function without breaking the build on Android.
 * Once Android starts supporting std::to_string function, this file and
 * AndroidCompat.cpp files can be discarded.
 */

#if defined(__ANDROID__)

#include <string>

namespace detail_AndroidCompat
{
    std::string to_string(int val);
    std::string to_string(unsigned val);
    std::string to_string(long val);
    std::string to_string(unsigned long val);
    std::string to_string(long long val);
    std::string to_string(unsigned long long val);
    std::string to_string(float val);
    std::string to_string(double val);
    std::string to_string(long double val);
}

namespace std
{
    using namespace detail_AndroidCompat;
}

#endif // __ANDROID__
#endif // ANDROIDCOMPAT_H
