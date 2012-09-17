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

#ifndef USMODULEIMPORT_H
#define USMODULEIMPORT_H

#include <usConfig.h>

#include <string>

US_BEGIN_NAMESPACE

struct ModuleActivator;

US_END_NAMESPACE

/**
 * \ingroup MicroServices
 *
 * \brief Import a static module.
 *
 * \param _import_module_libname The physical name of the module to import, without prefix or suffix.
 *
 * This macro imports the static module named \c _import_module_libname.
 *
 * Inserting this macro into your application's source code will allow you to make use of
 * a static module. Do not forget to list the imported module when calling the macro
 * #US_LOAD_IMPORTED_MODULES and to actually link the static module to the importing
 * executable or shared library.
 *
 * Example:
 * \snippet uServices-staticmodules/main.cpp ImportStaticModuleIntoLib
 *
 * \sa US_LOAD_IMPORTED_MODULES
 * \sa \ref MicroServices_StaticModules
 */
#define US_IMPORT_MODULE(_import_module_libname)                      \
  extern "C" US_PREPEND_NAMESPACE(ModuleActivator)* _us_module_activator_instance_ ## _import_module_libname (); \
  void _dummy_reference_to_ ## _import_module_libname ## _activator() \
  {                                                                   \
    _us_module_activator_instance_ ## _import_module_libname();       \
  }

/**
 * \ingroup MicroServices
 * \def US_LOAD_IMPORTED_MODULES_INTO_MAIN(_static_modules)
 *
 * \brief Import a list of static modules into an executable.
 *
 * \param _static_modules A space-deliminated list of physical module names, without prefix
 *        or suffix.
 *
 * This macro ensures that the ModuleActivator::Load(ModuleContext*) function is called
 * for each imported static module when the importing executable is loaded.
 *
 * There must be exactly one call of this macro in the executable which is
 * importing static modules.
 *
 * Example:
 * \snippet uServices-staticmodules/main.cpp ImportStaticModuleIntoMain
 *
 * \sa US_IMPORT_MODULE
 * \sa US_LOAD_IMPORTED_MODULES
 * \sa \ref MicroServices_StaticModules
 */

#ifdef US_BUILD_SHARED_LIBS
#define US_LOAD_IMPORTED_MODULES_INTO_MAIN(_static_modules)                            \
  extern "C" US_ABI_EXPORT const char* _us_get_imported_modules_for_()                 \
  {                                                                                    \
    return #_static_modules;                                                           \
  }
#else
#define US_LOAD_IMPORTED_MODULES_INTO_MAIN(_static_modules)                            \
  extern "C" US_ABI_EXPORT const char* _us_get_imported_modules_for_CppMicroServices() \
  {                                                                                    \
    return #_static_modules;                                                           \
  }
#endif

/**
 * \ingroup MicroServices
 *
 * \brief Import a list of static modules into a shared library.
 *
 * \param _module_libname The physical name of the importing module, without prefix or suffix.
 * \param _static_modules A space-deliminated list of physical module names, without prefix
 *        or suffix.
 *
 * This macro ensures that the ModuleActivator::Load(ModuleContext*) function is called
 * for each imported static module when the importing shared library is loaded.
 *
 * There must be exactly one call of this macro in the shared library which is
 * importing static modules.
 *
 * Example:
 * \snippet uServices-staticmodules/main.cpp ImportStaticModuleIntoLib
 *
 * \sa US_IMPORT_MODULE
 * \sa US_LOAD_IMPORTED_MODULES_INTO_MAIN
 * \sa \ref MicroServices_StaticModules
 */
#define US_LOAD_IMPORTED_MODULES(_module_libname, _static_modules)                         \
  extern "C" US_ABI_EXPORT const char* _us_get_imported_modules_for_ ## _module_libname () \
  {                                                                                        \
    return #_static_modules;                                                               \
  }

#endif // USMODULEREGISTRY_H
