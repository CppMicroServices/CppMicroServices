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

#include <usStaticInit.h>

#include <usModuleRegistry.h>
#include <usModuleContext.h>
#include <usModule.h>
#include <usModuleInfo.h>
#include <usModuleUtils.h>

US_BEGIN_NAMESPACE

US_GLOBAL_STATIC_WITH_ARGS(ModuleInfo, moduleInfo, ("@US_MODULE_NAME@", "@US_MODULE_LIBRARY_NAME@", "@US_MODULE_DEPENDS_STR@", "@US_MODULE_VERSION@"))

class US_ABI_LOCAL ModuleInitializer {

public:

  ModuleInitializer()
  {
    std::string location = ModuleUtils::GetLibraryPath(moduleInfo()->libName, reinterpret_cast<void*>(moduleInfo));
    std::string activator_func = "_us_module_activator_instance_";
    activator_func.append(moduleInfo()->name);

    moduleInfo()->location = location;

    if (moduleInfo()->libName.empty())
    {
      // make sure we retrieve symbols from the executable, if "libName" is empty
      location.clear();
    }
    moduleInfo()->activatorHook = reinterpret_cast<ModuleInfo::ModuleActivatorHook>(ModuleUtils::GetSymbol(location, activator_func.c_str()));

    Register();
  }

  static void Register()
  {
    ModuleRegistry::Register(moduleInfo());
  }

  ~ModuleInitializer()
  {
    ModuleRegistry::UnRegister(moduleInfo());
  }

};

ModuleContext* GetModuleContext()
{
  // make sure the module is registered
  if (moduleInfo()->id == 0)
  {
    ModuleInitializer::Register();
  }

  return ModuleRegistry::GetModule(moduleInfo()->id)->GetModuleContext();
}

US_END_NAMESPACE

static US_PREPEND_NAMESPACE(ModuleInitializer) coreModule;



