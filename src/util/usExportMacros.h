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


#ifndef USEXPORTMACROS_H
#define USEXPORTMACROS_H

#include <usConfig.h>

/**
  * Macros for import/export declarations
  */
#if defined(WIN32)
  #define US_ABI_EXPORT __declspec(dllexport)
  #define US_ABI_IMPORT __declspec(dllimport)
  #define US_ABI_LOCAL
#else
  #if __GNUC__ >= 4
    #define US_ABI_EXPORT __attribute__ ((visibility ("default")))
    #define US_ABI_IMPORT __attribute__ ((visibility ("default")))
    #define US_ABI_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define US_ABI_EXPORT
    #define US_ABI_IMPORT
    #define US_ABI_LOCAL
  #endif
#endif

#ifdef US_BUILD_SHARED_LIBS
  // We are building a shared lib
  #ifdef CppMicroServices_EXPORTS
    #define US_EXPORT US_ABI_EXPORT
  #else
    #define US_EXPORT US_ABI_IMPORT
  #endif
#else
  // We are building a static lib
  #if __GNUC__ >= 4
    // Don't hide RTTI symbols of definitions in the C++ Micro Services
    // headers that are included in DSOs with hidden visibility
    #define US_EXPORT US_ABI_EXPORT
  #else
    #define US_EXPORT
  #endif
#endif

#endif // USEXPORTMACROS_H
