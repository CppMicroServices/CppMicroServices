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


#ifndef CPPMICROSERVICES_UTILS_H
#define CPPMICROSERVICES_UTILS_H

#include "cppmicroservices/FrameworkExport.h"

#include <string>


namespace cppmicroservices {

//-------------------------------------------------------------------
// File type checking
//-------------------------------------------------------------------

bool IsSharedLibrary(const std::string& location);

bool IsBundleFile(const std::string& location);

//-------------------------------------------------------------------
// Framework storage
//-------------------------------------------------------------------

class CoreBundleContext;

extern const std::string FWDIR_DEFAULT;

std::string GetFrameworkDir(CoreBundleContext* ctx);

/**
 * Check for local file storage directory.
 *
 * @return A directory path or an empty string if no storage is available.
 */
std::string GetFileStorage(CoreBundleContext* ctx, const std::string& name, bool create = true);

//-------------------------------------------------------------------
// Generic utility functions
//-------------------------------------------------------------------

void TerminateForDebug(const std::exception_ptr ex);

namespace detail {
US_Framework_EXPORT std::string GetDemangledName(const std::type_info& typeInfo);
}


} // namespace cppmicroservices

#endif // CPPMICROSERVICES_UTILS_H
