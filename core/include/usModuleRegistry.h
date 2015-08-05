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

#ifndef USMODULEREGISTRY_H
#define USMODULEREGISTRY_H

#include <vector>
#include <string>

#include <usCoreConfig.h>
#include <usThreads_p.h>

US_BEGIN_NAMESPACE

class CoreModuleContext;
class Module;
struct ModuleInfo;
struct ModuleActivator;

/**
 * \ingroup MicroServices
 *
 * Here we handle all the modules that are loaded in the framework.
 */
class US_Core_EXPORT ModuleRegistry {

public:
  
  ModuleRegistry(CoreModuleContext* coreCtx) : 
      coreCtx(coreCtx), 
      modulesLock(new Mutex()),
      countLock(new Mutex()),
      id(0)
  {
  
  }

  ~ModuleRegistry(void) {}

  /**
   * Get the module that has the specified module identifier.
   *
   * @param id The identifier of the module to get.
   * @return Module or null
   *         if the module was not found.
   */
  Module* GetModule(long id);

  /**
   * Get the module that has specified module name.
   *
   * @param name The name of the module to get.
   * @return Module or null.
   */
  Module* GetModule(const std::string& name);

  /**
   * Get all known modules.
   *
   * @return A list which is filled with all known modules.
   */
  std::vector<Module*> GetModules();

  /**
   * Get all modules currently in module state <code>LOADED</code>.
   *
   * @return A list which is filled with all modules in
   *         state <code>LOADED</code>
   */
  //std::vector<Module*> GetLoadedModules();

  Module* Register(ModuleInfo* info);

  Module* UnRegister(const ModuleInfo* info);

private:
  // don't allow copying the ModuleRegistry.
  ModuleRegistry(const ModuleRegistry& );
  ModuleRegistry& operator=(const ModuleRegistry& );

  CoreModuleContext* coreCtx;

  typedef US_UNORDERED_MAP_TYPE<std::string, Module*> ModuleMap;

  /**
   * Table of all installed modules in this framework.
   * Key is the module id.
   */
  ModuleMap modules;

  /**
   * Lock for protecting the modules object
   */
  Mutex* modulesLock;

  /**
   * Lock for protecting the register count
   */
  Mutex* countLock;

  /**
   * Stores the next Bundle ID.
   */
  long id;

};

US_END_NAMESPACE

#endif // USMODULEREGISTRY_H
