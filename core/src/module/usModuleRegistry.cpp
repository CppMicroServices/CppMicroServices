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

#include "usModuleRegistry.h"

#include "usModule.h"
#include "usModuleInfo.h"
#include "usModuleContext.h"
#include "usModuleActivator.h"
#include "usCoreModuleContext_p.h"
#include "usGetModuleContext.h"

#include <cassert>
#include <map>


US_BEGIN_NAMESPACE

Module* ModuleRegistry::Register(ModuleInfo* info)
{
  Module* module = 0;
  if (info->id > 0)
  {
    // The module was already registered
    {
      MutexLock lock(*modulesLock);
      module = modules[info->name];
      assert(module != 0);
    }
  }
  else
  {
    // check if the module is reloaded
    {
      MutexLock lock(*modulesLock);
      for (ModuleMap::const_iterator i = modules.begin();
           i != modules.end(); ++i)
      {
        if (i->second->GetLocation() == info->location &&
            i->second->GetName() == info->name)
        {
          module = i->second;
          info->id = module->GetModuleId();
        }
      }
    }

    if (!module)
    {
      module = new Module();
      countLock->Lock();
      info->id = ++id;
      assert(info->id == 1 ? info->name == "CppMicroServices" : true);
      countLock->Unlock();
      module->Init(coreCtx, info);

      MutexLock lock(*modulesLock);
      modules.insert(std::make_pair(info->name, module));
    }
  }

  return module;
}

Module* ModuleRegistry::UnRegister(const ModuleInfo* info)
{
  Module* curr = 0;
  if (info->id > 1)
  {
    MutexLock lock(*modulesLock);
    curr = modules[info->name];
    assert(curr != 0);
  }
  return curr;
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

//std::vector<Module*> ModuleRegistry::GetLoadedModules()
//{
//  MutexLock lock(*modulesLock);
//
//  std::vector<Module*> result;
//  ModuleMap::const_iterator iter = modules.begin();
//  ModuleMap::const_iterator iterEnd = modules.end();
//  for (; iter != iterEnd; ++iter)
//  {
//    if (iter->second->IsLoaded())
//    {
//      result.push_back(iter->second);
//    }
//  }
//  return result;
//}

US_END_NAMESPACE

