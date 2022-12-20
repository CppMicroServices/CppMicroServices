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

/*
  File: PropsCheck.h

  This file contains shared utility functions for validating that maps provided to
  the Property class are indeed valid (e.g., no pairs of keys which differ in case only).
  The code contained in this file has been extracted from other individual files to promote
  code reusability while reducing the amount of code duplication that exists so that maintenance
  is easier.

  Functions defined here are intended for internal use; no external clients of the core
  framework should be capable/allowed of calling these functions.
 */

#ifndef PROPSCHECK_H
#define PROPSCHECK_H

#include <stdexcept>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/LDAPFilter.h"

#ifdef US_PLATFORM_WINDOWS
#    include <string.h>
#    define ci_compare strnicmp
#else
#    include <strings.h>
#    define ci_compare strncasecmp
#endif

namespace cppmicroservices
{

    namespace props_check
    {
        /**
         * @brief Validates that the provided AnyMap conforms to the same constraints that
         * those stored in Property objects have.
         *
         * The provided AnyMap is said to be valid if there exists no pairs of two keys
         * which differ in case only (e.g., "service.feature", "Service.feature"). If this
         * condition is not true, this function throws as defined below.
         *
         * @param am The AnyMap to validate
         * @throws std::runtime_error Thrown when `am` is invalid (described above)
         */
        void ValidateAnyMap(cppmicroservices::AnyMap const& am);

        std::string ToLower(std::string const& s);
    } // namespace props_check
} // namespace cppmicroservices

#endif
