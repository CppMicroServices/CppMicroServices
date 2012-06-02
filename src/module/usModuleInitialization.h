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

#include <usStaticInit_p.h>

#include <usModuleRegistry.h>
#include <usModuleInfo.h>
#include <usModuleContext.h>
#include <usModule.h>
#include <usModuleUtils_p.h>

#ifndef USMODULEINITIALIZATION_H
#define USMODULEINITIALIZATION_H

/**
 * \ingroup MicroServices
 *
 * \def US_INITIALIZE_MODULE(_module_name, _module_libname, _module_depends, _module_version)
 * \brief Creates initialization code for a module.
 *
 * Each module which wants to register itself with the CppMicroServices library
 * has to put a call to this macro in one of its source files.
 *
 * Example call for a module with file-name "libmylibname.so".
 * \code
 * US_INITIALIZE_MODULE("My Service Implementation", "mylibname", "", "1.0.0")
 * \endcode
 *
 * \remarks If you are using CMake, consider using the provided CMake macro
 * <code>usFunctionGenerateModuleInit()</code>.
 *
 * \param _module_name A human-readable name for the module.
 *        If you use this macro in a source file for an executable, the module name must
 *        be a valid C-identifier (no spaces etc.).
 * \param _module_libname The physical name of the module, withou prefix or suffix.
 * \param _module_depends A list of module dependencies. This is meta-data only.
 * \param _module_version A version string in the form of "<major>.<minor>.<micro>.<qualifier>".
 */
#define US_INITIALIZE_MODULE(_module_name, _module_libname, _module_depends, _module_version) \
US_BEGIN_NAMESPACE \
\
/* Declare a file scoped ModuleInfo object */ \
US_GLOBAL_STATIC_WITH_ARGS(ModuleInfo, moduleInfo, (_module_name, _module_libname, _module_depends, _module_version)) \
\
/* This class is used to statically initialize the library within the C++ Micro services \
   library. looks up a library specific C-style function a */ \
class US_ABI_LOCAL ModuleInitializer { \
\
public: \
\
  ModuleInitializer() \
  { \
    std::string location = ModuleUtils::GetLibraryPath(moduleInfo()->libName, \
                                                       reinterpret_cast<void*>(moduleInfo)); \
    std::string activator_func = "_us_module_activator_instance_"; \
    if(moduleInfo()->libName.empty()) \
    { \
      activator_func.append(moduleInfo()->name); \
    } \
    else \
    { \
      activator_func.append(moduleInfo()->libName); \
    } \
\
    moduleInfo()->location = location;\
\
    if (moduleInfo()->libName.empty()) \
    { \
      /* make sure we retrieve symbols from the executable, if "libName" is empty */ \
      location.clear(); \
    } \
    moduleInfo()->activatorHook = reinterpret_cast<ModuleInfo::ModuleActivatorHook>(ModuleUtils::GetSymbol(location, activator_func.c_str())); \
\
    Register(); \
  } \
\
  static void Register() \
  { \
    ModuleRegistry::Register(moduleInfo()); \
  } \
\
  ~ModuleInitializer() \
  { \
    ModuleRegistry::UnRegister(moduleInfo()); \
  } \
\
}; \
\
US_ABI_LOCAL ModuleContext* GetModuleContext() \
{ \
  /* make sure the module is registered */ \
  if (moduleInfo()->id == 0) \
  { \
    ModuleInitializer::Register(); \
  } \
\
  return ModuleRegistry::GetModule(moduleInfo()->id)->GetModuleContext(); \
} \
\
US_END_NAMESPACE \
\
static US_PREPEND_NAMESPACE(ModuleInitializer) _InitializeModule; \


#endif // USMODULEINITIALIZATION_H
