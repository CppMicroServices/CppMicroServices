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

#include <usConfig.h>

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
   * @param modules A list which is filled with all known modules.
   */
  static void GetModules(std::vector<Module*>& modules);

  /**
   * Get all modules currently in module state <code>LOADED</code>.
   *
   * @param modules A list which is filled with all modules in
   *        state <code>LOADED</code>
   */
  static void GetLoadedModules(std::vector<Module*>& modules);

private:

  friend class ModuleInitializer;

  // disabled
  ModuleRegistry();

  static void Register(ModuleInfo* info);

  static void UnRegister(const ModuleInfo* info);

};

US_END_NAMESPACE

#endif // USMODULEREGISTRY_H
