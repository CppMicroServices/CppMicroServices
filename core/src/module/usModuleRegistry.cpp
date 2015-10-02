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

#include "usModuleRegistry_p.h"

#include "usFramework.h"
#include "usModule.h"
#include "usModuleInfo.h"
#include "usModuleContext.h"
#include "usModuleActivator.h"
#include "usModulePrivate.h"
#include "usCoreModuleContext_p.h"
#include "usGetModuleContext.h"

#include <cassert>
#include <map>


US_BEGIN_NAMESPACE

ModuleRegistry::ModuleRegistry(CoreModuleContext* coreCtx) :
  coreCtx(coreCtx)
{
  id.value = 0;
}

ModuleRegistry::~ModuleRegistry(void)
{
}

Module* ModuleRegistry::Register(ModuleInfo* info)
{
  Module* module = GetModule(info->name);

  if (!module)
  {
    module = new Module();
    {
      Lock{id};
      info->id = ++id.value;
      assert(info->id == 1 ? info->name == "CppMicroServices" : true);
    }
    module->Init(coreCtx, info);

    Lock(this);
    std::pair<ModuleMap::iterator, bool> return_pair(modules.insert(std::make_pair(info->name, module)));

    // A race condition exists when creating a new bundle instance. To resolve
    // this requires either scoping the mutex to the entire function or adding
    // additional logic to check for duplicates on insert into the bundle registry.
    // Otherwise, an "orphan" bundle instance could be returned to the caller,
    // which isn't actually contained within the bundle registry.
    //
    // Based on the bundle registry performance unit test, deleting the
    // orphaned bundle instance is faster than increasing the scope of the
    // mutex.
    if (!return_pair.second)
    {
      ModuleMap::iterator iter(return_pair.first);
      delete module;
      module = (*iter).second;
    }    
  }

  return module;
}

void ModuleRegistry::RegisterSystemBundle(Framework* const systemBundle, ModuleInfo* info)
{
  if (!systemBundle)
  {
    throw std::invalid_argument("Can't register a null system bundle");
  }

  {
    Lock{id};
    info->id = ++id.value;
    assert(info->id == 1 ? info->name == "CppMicroServices" : true);
  }

  systemBundle->Init(coreCtx, info);

  Lock(this);
  modules.insert(std::make_pair(info->name, systemBundle));
}

void ModuleRegistry::UnRegister(const ModuleInfo* info)
{
  // TODO: fix once the system bundle id is set to 0
  if (info->id > 1)
  {
    Lock(this);
    modules.erase(info->name);
  }
}

Module* ModuleRegistry::GetModule(long id) const
{
  Lock(this);

  for (auto& m : modules)
  {
    if (m.second->GetModuleId() == id)
    {
      return m.second;
    }
  }
  return 0;
}

Module* ModuleRegistry::GetModule(const std::string& name) const
{
  Lock(this);

  auto iter = modules.find(name);
  if (iter != modules.end())
  {
    return iter->second;
  }
  return 0;
}

std::vector<Module*> ModuleRegistry::GetModules() const
{
  Lock(this);

  std::vector<Module*> result;
  for (auto& m : modules)
  {
    result.push_back(m.second);
  }
  return result;
}

US_END_NAMESPACE

