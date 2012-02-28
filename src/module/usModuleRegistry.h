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

#ifndef USMODULEREGISTRY_H
#define USMODULEREGISTRY_H

#include <vector>
#include <string>

#include <usExportMacros.h>

US_BEGIN_NAMESPACE

class Module;
struct ModuleInfo;
struct ModuleActivator;

/**
 * \ingroup MicroServices
 *
 * Here we handle all the modules that are loaded in the framework.
 */
class US_EXPORT ModuleRegistry {

public:

  /**
   * Get the module that has the specified module identifier.
   *
   * @param id The identifier of the module to get.
   * @return Module or null
   *         if the module was not found.
   */
  static Module* GetModule(long id);

  /**
   * Get the module that has specified module name.
   *
   * @param name The name of the module to get.
   * @return Module or null.
   */
  static Module* GetModule(const std::string& name);

  /**
   * Get all known modules.
   *
   * @return A Module list with modules.
   */
  static void GetModules(std::vector<Module*>& modules);

  /**
   * Get all modules currently in module state LOADED.
   *
   * @return A List of Modules.
   */
  static void GetLoadedModules(std::vector<Module*>& modules);

private:

  friend class ModuleInitializer;

  // disabled
  ModuleRegistry();

  static void Register(ModuleInfo* info);

  static void UnRegister(const ModuleInfo* info);

};

typedef ModuleActivator* (*ModuleActivatorInstanceFunction)();

void US_EXPORT RegisterStaticModuleActivatorInstanceFunction(ModuleActivatorInstanceFunction func);
void US_EXPORT UnregisterStaticModuleActivatorInstanceFunction(ModuleActivatorInstanceFunction func);

US_END_NAMESPACE

#define US_MODULE_LOAD(moduleName, context) \
  _us_module_activator_instance_ ## moduleName ()->Load(context);

#define US_MODULE_UNLOAD(moduleName, context) \
  _us_module_activator_instance_ ## moduleName ()->Unload(context);

#define US_MODULE_IMPORT(moduleName)                                                             \
  extern ::US_PREPEND_NAMESPACE(ModuleActivator)* _us_module_activator_instance_##moduleName();  \
  class Static##moduleName##ModuleInstance                                                       \
  {                                                                                              \
  public:                                                                                        \
    Static##moduleName##ModuleInstance()                                                         \
    {                                                                                            \
      RegisterStaticModuleActivatorInstanceFunction(_us_module_activator_instance_##moduleName); \
      _us_module_activator_instance_##moduleName()->Load(::US_PREPEND_NAMESPACE(GetModuleContext)()); \
    }                                                                                            \
    ~Static##moduleName##ModuleInstance()                                                        \
    {                                                                                            \
      UnregisterStaticModuleActivatorInstanceFunction(_us_module_activator_instance_##moduleName); \
      _us_module_activator_instance_##moduleName()->Unload(::US_PREPEND_NAMESPACE(GetModuleContext)()); \
    }                                                                                            \
  };                                                                                             \
  static Static##moduleName##ModuleInstance static##moduleName##Instance;


#endif // USMODULEREGISTRY_H
