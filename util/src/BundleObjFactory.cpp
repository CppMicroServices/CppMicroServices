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

#include "cppmicroservices/util/BundleObjFactory.h"
#include "cppmicroservices/util/BundleObjFile.h"
#include "cppmicroservices/util/BundlePEFile.h"
#include "cppmicroservices/util/BundleElfFile.h"
#include "cppmicroservices/util/BundleMachOFile.h"

namespace cppmicroservices {

/// Return a BundleObjFile which represents data read from the binary at
/// the given location.
///
/// @param location absolute path to a PE, ELF or Mach-O binary file.
/// @return A BundleObjFile object
/// @throws If location is not a valid PE, ELF or Mach-O binary file.
///
/// @note The location must be a valid binary format for the host machine.
///       i.e. PE file on Windows, Mach-O on macOS, ELF on Linux
std::unique_ptr<BundleObjFile> BundleObjFactory::CreateBundleFileObj(const std::string& location)
{
#if defined(US_PLATFORM_WINDOWS)
  return CreateBundlePEFile(location);
#elif defined(US_PLATFORM_APPLE)
  return CreateBundleMachOFile(location);
#elif defined(US_PLATFORM_LINUX)
  return CreateBundleElfFile(location);
#else
  #error "Unknown OS platform";
#endif
  return {};
}   
};
