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

#include <cstring>

#ifndef USMODULEINITIALIZATION_H
#define USMODULEINITIALIZATION_H

/**
 * \ingroup MicroServices
 *
 * \brief Creates initialization code for a module.
 *
 * Each module which wants to register itself with the CppMicroServices library
 * has to put a call to this macro in one of its source files.
 *
 * Example call for a module with file-name "libmylibname.so":
 * \code
 * US_INITIALIZE_MODULE("My Service Implementation", "mylibname")
 * \endcode
 *
 * This will initialize the module for use with the CppMicroServices library, using a default
 * auto-load directory named after the provided library name in \c _module_libname.
 *
 * \sa MicroServices_AutoLoading
 *
 * \remarks If you are using CMake, consider using the provided CMake macro
 * <code>usFunctionGenerateModuleInit()</code>.
 *
 * \param _module_name A human-readable name for the module.
 *        If you use this macro in a source file for an executable, the module name must
 *        be a valid C-identifier (no spaces etc.).
 * \param _module_libname The physical name of the module, withou prefix or suffix.
 *
 * \note If you want to create initialization code for an executable, see
 * #US_INITIALIZE_EXECUTABLE.
 */
#define US_INITIALIZE_MODULE(_module_name, _module_libname)                                  \
US_BEGIN_NAMESPACE                                                                           \
                                                                                             \
/* Declare a file scoped ModuleInfo object */                                                \
US_GLOBAL_STATIC_WITH_ARGS(ModuleInfo, moduleInfo, (_module_name, _module_libname))          \
                                                                                             \
/* This class is used to statically initialize the library within the C++ Micro services     \
   library. It looks up a library specific C-style function returning an instance            \
   of the ModuleActivator interface. */                                                      \
class US_ABI_LOCAL ModuleInitializer {                                                       \
                                                                                             \
public:                                                                                      \
                                                                                             \
  ModuleInitializer()                                                                        \
  {                                                                                          \
    ModuleInfo*(*moduleInfoPtr)() = moduleInfo;                                              \
    void* moduleInfoSym = NULL;                                                              \
    std::memcpy(&moduleInfoSym, &moduleInfoPtr, sizeof(void*));                              \
    std::string location = ModuleUtils::GetLibraryPath(moduleInfo()->libName, moduleInfoSym); \
    std::string activator_func = "_us_module_activator_instance_";                           \
    if(moduleInfo()->libName.empty())                                                        \
    {                                                                                        \
      activator_func.append(moduleInfo()->name);                                             \
    }                                                                                        \
    else                                                                                     \
    {                                                                                        \
      activator_func.append(moduleInfo()->libName);                                          \
    }                                                                                        \
                                                                                             \
    moduleInfo()->location = location;                                                       \
                                                                                             \
    if (moduleInfo()->libName.empty())                                                       \
    {                                                                                        \
      /* make sure we retrieve symbols from the executable, if "libName" is empty */         \
      location.clear();                                                                      \
    }                                                                                        \
    void* activatorHookSym = ModuleUtils::GetSymbol(location, activator_func.c_str());       \
    std::memcpy(&moduleInfo()->activatorHook, &activatorHookSym, sizeof(void*));             \
                                                                                             \
    Register();                                                                              \
  }                                                                                          \
                                                                                             \
  static void Register()                                                                     \
  {                                                                                          \
    ModuleRegistry::Register(moduleInfo());                                                  \
  }                                                                                          \
                                                                                             \
  ~ModuleInitializer()                                                                       \
  {                                                                                          \
    ModuleRegistry::UnRegister(moduleInfo());                                                \
  }                                                                                          \
                                                                                             \
};                                                                                           \
                                                                                             \
US_ABI_LOCAL ModuleContext* GetModuleContext()                                               \
{                                                                                            \
  /* make sure the module is registered */                                                   \
  if (moduleInfo()->id == 0)                                                                 \
  {                                                                                          \
    ModuleInitializer::Register();                                                           \
  }                                                                                          \
                                                                                             \
  return ModuleRegistry::GetModule(moduleInfo()->id)->GetModuleContext();                    \
}                                                                                            \
                                                                                             \
US_END_NAMESPACE                                                                             \
                                                                                             \
static US_PREPEND_NAMESPACE(ModuleInitializer) _InitializeModule;

/**
 * \ingroup MicroServices
 *
 * \brief Creates initialization code for an executable.
 *
 * Each executable which wants to register itself with the CppMicroServices library
 * has to put a call to this macro in one of its source files. This ensures that the
 * executable will get its own ModuleContext instance and can access the service registry.
 *
 * Example call for an executable:
 * \code
 * US_INITIALIZE_EXECUTABLE("my_executable")
 * \endcode
 *
 * This will initialize the executable for use with the CppMicroServices library, using a default
 * auto-load directory named after the provided executable id in \c _executable_id.
 *
 * \sa MicroServices_AutoLoading
 *
 * \remarks If you are using CMake, consider using the provided CMake macro
 * <code>usFunctionGenerateExecutableInit()</code>.
 *
 * \param _executable_id A valid C identifier for the executable (no spaces etc.).
 */
#define US_INITIALIZE_EXECUTABLE(_executable_id) \
  US_INITIALIZE_MODULE(_executable_id, "")

// If the CppMicroServices library was statically build, executables will share the
// initialization code with the CppMicroServices library.
#ifndef US_BUILD_SHARED_LIBS
#undef US_INITIALIZE_EXECUTABLE
#define US_INITIALIZE_EXECUTABLE(_a)
#endif

// Static modules usually don't get initialization code. They are initialized within the
// module importing the static module(s).
#if defined(US_STATIC_MODULE) && !defined(US_FORCE_MODULE_INIT)
#undef US_INITIALIZE_MODULE
#define US_INITIALIZE_MODULE(_a,_b)
#endif

#endif // USMODULEINITIALIZATION_H
