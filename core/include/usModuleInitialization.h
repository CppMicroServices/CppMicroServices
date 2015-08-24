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

#ifndef US_MODULE_NAME
#error Missing US_MODULE_NAME preprocessor define
#endif

#ifndef USMODULEINITIALIZATION_H
#define USMODULEINITIALIZATION_H

#include <usGlobalConfig.h>
#include <usModuleContext.h>
#include <usModuleUtils_p.h>

/**
 * \ingroup MicroServices
 *
 * \brief Creates initialization code for a module.
 *
 * Each module which wants to register itself with the CppMicroServices library
 * has to put a call to this macro in one of its source files. Further, the module's
 * source files must be compiled with the \c US_MODULE_NAME pre-processor definition
 * set to a module-unique identifier.
 *
 * Calling the \c US_INITIALIZE_MODULE macro will initialize the module for use with
 * the CppMicroServices library, using a default auto-load directory named after the
 * \c US_MODULE_NAME definition.
 *
 * \sa MicroServices_AutoLoading
 *
 * \remarks If you are using CMake, consider using the provided CMake macro
 * <code>usFunctionGenerateModuleInit()</code>.
 */
#define US_INITIALIZE_MODULE                                                                                                     \
    US_PREPEND_NAMESPACE(ModuleContext)* US_CONCAT(_us_bundle_context_instance_, US_MODULE_NAME) = 0;                            \
                                                                                                                                 \
    extern "C" US_ABI_EXPORT US_PREPEND_NAMESPACE(ModuleContext)* US_CONCAT(_us_get_bundle_context_instance_, US_MODULE_NAME) () \
    {                                                                                                                            \
        return US_CONCAT(_us_bundle_context_instance_, US_MODULE_NAME);                                                          \
    }                                                                                                                            \
                                                                                                                                 \
    extern "C" US_ABI_EXPORT void US_CONCAT(_us_set_bundle_context_instance_, US_MODULE_NAME) (US_PREPEND_NAMESPACE(ModuleContext)* ctx) \
    {                                                                                                                                    \
        US_CONCAT(_us_bundle_context_instance_, US_MODULE_NAME) = ctx;                                                                   \
    }


#endif // USMODULEINITIALIZATION_H
