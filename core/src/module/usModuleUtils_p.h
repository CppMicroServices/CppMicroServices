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


#ifndef USMODULEUTILS_H
#define USMODULEUTILS_H

#include <usCoreExport.h>

#include <string>

#define US_STR_(x) #x
#define US_STR(x) US_STR_(x)
#define US_CONCAT_(x,y) x ## y
#define US_CONCAT(x,y) US_CONCAT_(x,y)

US_BEGIN_NAMESPACE

struct ModuleInfo;

/**
 * This class is not intended to be used directly. It is exported to support
 * the CppMicroServices module system.
 */
struct US_Core_EXPORT ModuleUtils
{
  static std::string GetLibraryPath(void* symbol);

  static void* GetSymbol(const ModuleInfo& module, const char* symbol);
};

US_END_NAMESPACE

#endif // USMODULEUTILS_H
