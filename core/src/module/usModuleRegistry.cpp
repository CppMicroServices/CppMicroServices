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
    coreCtx(coreCtx),
    modulesLock(new Mutex()),
    countLock(new Mutex()),
    id(0)
{

}

ModuleRegistry::~ModuleRegistry(void)
{
    if (modulesLock)
    {
        delete modulesLock;
    }

    if (countLock)
    {
        delete countLock;
    }

    id = 0;
}

Module* ModuleRegistry::Register(ModuleInfo* info)
{
  Module* module = GetModule(info->name);

  if (!module)
  {
    module = new Module();
    countLock->Lock();
    info->id = ++id;
    assert(info->id == 1 ? info->name == "CppMicroServices" : true);
    countLock->Unlock();
    module->Init(coreCtx, info);

    MutexLock lock(*modulesLock);
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

  countLock->Lock();
  info->id = ++id;
  assert(info->id == 1 ? info->name == "CppMicroServices" : true);
  countLock->Unlock();

  systemBundle->Init(coreCtx, info);

  MutexLock lock(*modulesLock);
  modules.insert(std::make_pair(info->name, systemBundle));
}

void ModuleRegistry::UnRegister(const ModuleInfo* info)
{
  // TODO: fix once the system bundle id is set to 0
  if (info->id > 1)
  {
    MutexLock lock(*modulesLock);
    modules.erase(info->name);
  }
}

Module* ModuleRegistry::GetModule(long id)
{
  MutexLock lock(*modulesLock);

  ModuleMap::const_iterator iter = modules.begin();
  ModuleMap::const_iterator iterEnd = modules.end();
  for (; iter != iterEnd; ++iter)
  {
    if (iter->second->GetModuleId() == id)
    {
      return iter->second;
    }
  }
  return 0;
}

Module* ModuleRegistry::GetModule(const std::string& name)
{
  MutexLock lock(*modulesLock);

  ModuleMap::const_iterator iter = modules.find(name);
  if (iter != modules.end())
  {
    return iter->second;
  }
  return 0;
}

std::vector<Module*> ModuleRegistry::GetModules()
{
  MutexLock lock(*modulesLock);

  std::vector<Module*> result;
  ModuleMap::const_iterator iter = modules.begin();
  ModuleMap::const_iterator iterEnd = modules.end();
  for (; iter != iterEnd; ++iter)
  {
    result.push_back(iter->second);
  }
  return result;
}

US_END_NAMESPACE

