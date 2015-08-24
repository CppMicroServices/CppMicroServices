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

#ifndef USMODULEREGISTRY_P_H
#define USMODULEREGISTRY_P_H

#include <vector>
#include <string>

#include <usCoreConfig.h>
#include <usThreads_p.h>

US_BEGIN_NAMESPACE

class CoreModuleContext;
class Framework;
class Module;
struct ModuleInfo;
struct ModuleActivator;

/**
 * Here we handle all the modules that are loaded in the framework.
 * @remarks This class is thread-safe.
 */
class US_Core_EXPORT ModuleRegistry {

public:
  
  ModuleRegistry(CoreModuleContext* coreCtx);
  virtual ~ModuleRegistry(void);

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
   * Register a bundle with the Framework
   *
   * @return The registered bundle.
   */
  Module* Register(ModuleInfo* info);
  
  /**
   * Register the system bundle.
   *
   * A helper function to help bootstrap the Framework.
   *
   * @param systemBundle The system bundle to register.
   */
  void RegisterSystemBundle(Framework* const systemBundle, ModuleInfo* info);

  /**
   * Remove a bundle from the Framework.
   *
   * Register(ModuleInfo* info) must be called to re-install the bundle. 
   * Upon which, the bundle will receive a new unique bundle id.
   *
   */
  void UnRegister(const ModuleInfo* info);

private:
  // don't allow copying the ModuleRegistry.
  ModuleRegistry(const ModuleRegistry& );
  ModuleRegistry& operator=(const ModuleRegistry& );

  CoreModuleContext* coreCtx;

  typedef US_UNORDERED_MAP_TYPE<std::string, Module*> ModuleMap;

  /**
   * Table of all installed modules in this framework.
   * Key is the module name.
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

#endif // USMODULEREGISTRY_P_H
