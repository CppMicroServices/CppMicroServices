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


#ifndef USBUNDLEUTILS_H
#define USBUNDLEUTILS_H

#include <usCoreExport.h>

#include <string>

namespace us {

namespace BundleUtils
{
  /**
   * Get the absolute file path of the shared library or executable that
   * provides a definition of the given symbol.
   *
   * @param symbol The symbol to look for.
   * @return The absolute file path of the library or executable containing a definition of
   *         \c symbol. An empty string if not found.
   */
  US_Core_EXPORT std::string GetLibraryPath(void* symbol);

  /**
   * Get the address for a symbol.
   *
   * @param bundleName The name of the bundle defining the symbol.
   * @param libLocation The absolute file location of the bundle defining the symbol.
   * @param symbol The symbol name to look for.
   * @return the address of the \c symbol. A <code>nullptr</code> if the symbol
   *         wasn't found.
   */
  US_Core_EXPORT void* GetSymbol(const std::string& bundleName, const std::string& libLocation, const char* symbol);
}

}

#endif // USBUNDLEUTILS_H
