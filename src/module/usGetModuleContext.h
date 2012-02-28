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


#ifndef USGETMODULECONTEXT_H
#define USGETMODULECONTEXT_H

#include <usExportMacros.h>

US_BEGIN_NAMESPACE

class ModuleContext;

/**
 * Returns the module context of the calling module.
 *
 * This function allows easy access to the own ModuleContext instance from
 * inside a US module.
 *
 * \internal
 *
 * Note that the corresponding function definition is provided for each
 * module by the mitkModuleInit.cpp file. This file is customized via CMake
 * configure_file(...) and automatically compiled into each US module.
 * Consequently, the GetModuleContext function is not exported, since each
 * module gets its "own version".
 */
US_ABI_LOCAL ModuleContext* GetModuleContext();

US_END_NAMESPACE

#endif // USGETMODULECONTEXT_H
