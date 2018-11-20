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

#ifndef CPPMICROSERVICES_BUNDLEOBJFACTORY_H
#define CPPMICROSERVICES_BUNDLEOBJFACTORY_H

#include "BundleObjFile.h"

namespace cppmicroservices {

class BundleObjFactory
{
public:
    BundleObjFactory() = default;
    
    /// Return a BundleObjFile which represents data read from the binary at
    /// the given location.
    ///
    /// @param location absolute path to a PE, ELF or Mach-O binary file.
    /// @return A BundleObjFile object
    /// @throws If location is not a valid PE, ELF or Mach-O binary file.
    ///
    /// @note The location must be a valid binary format for the host machine.
    ///       i.e. PE file on Windows, Mach-O on macOS, ELF on Linux
    std::unique_ptr<BundleObjFile> CreateBundleFileObj(const std::string& location);
    
};

}

#endif /* CPPMICROSERVICES_BUNDLEOBJFACTORY_H */
