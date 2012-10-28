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

#include <usConfig.h>

#include "usModuleRegistry.h"

#include "usModule.h"
#include "usModuleInfo.h"
#include "usModuleContext.h"
#include "usModuleActivator.h"
#include "usCoreModuleContext_p.h"
#include "usGetModuleContext.h"
#include "usStaticInit_p.h"

#include <cassert>
#include <map>


US_BEGIN_NAMESPACE

typedef Mutex MutexType;
typedef MutexLock<MutexType> MutexLocker;

typedef US_UNORDERED_MAP_TYPE<long, Module*> ModuleMap;

US_GLOBAL_STATIC(CoreModuleContext, coreModuleContext)

template<typename T>
struct ModuleDeleter
{
  void operator()(GlobalStatic<T>& globalStatic) const
  {
    ModuleMap* moduleMap = globalStatic.pointer;
    for (ModuleMap::const_iterator i = moduleMap->begin();
         i != moduleMap->end(); ++i)
    {
      delete i->second;
    }
    DefaultGlobalStaticDeleter<T> defaultDeleter;
    defaultDeleter(globalStatic);
  }
};

/**
 * Table of all installed modules in this framework.
 * Key is the module id.
 */
US_GLOBAL_STATIC_WITH_DELETER(ModuleMap, modules, ModuleDeleter)

/**
 * Lock for protecting the modules object
 */
US_GLOBAL_STATIC(MutexType, modulesLock)

/**
 * Lock for protecting the register count
 */
US_GLOBAL_STATIC(MutexType, countLock)


void ModuleRegistry::Register(ModuleInfo* info)
{
  static long regCount = 0;
  if (info->id > 0)
  {
    // The module was already registered
    Module* module = 0;
    {
      MutexLocker lock(*modulesLock());
      module = modules()->operator[](info->id);
      assert(module != 0);
    }
    module->Start();
  }
  else
  {
    Module* module = 0;
    // check if the module is reloaded
    {
      MutexLocker lock(*modulesLock());
      ModuleMap* map = modules();
      for (ModuleMap::const_iterator i = map->begin();
           i != map->end(); ++i)
      {
        if (i->second->GetLocation() == info->location)
        {
          module = i->second;
          info->id = module->GetModuleId();
        }
      }
    }

    if (!module)
    {
      module = new Module();
      countLock()->Lock();
      info->id = ++regCount;
      countLock()->Unlock();

      module->Init(coreModuleContext(), info);

      MutexLocker lock(*modulesLock());
      ModuleMap* map = modules();
      map->insert(std::make_pair(info->id, module));
    }
    else
    {
      module->Init(coreModuleContext(), info);
    }

    module->Start();
  }
}

void ModuleRegistry::UnRegister(const ModuleInfo* info)
{
  // If we are unregistering the core module, we just call
  // the module activators Unload() method (if there is a
  // module activator). Since we cannot be sure that the
  // ModuleContext for the core library is still valid, we
  // just pass a null-pointer. Using the module context during
  // static deinitalization time of the core library makes
  // no sense anyway.
  if (info->id == 1)
  {
    // Remove listeners from static modules if they have forgotten to do so
    coreModuleContext()->listeners.RemoveAllListeners(GetModuleContext());

    if (info->activatorHook)
    {
      info->activatorHook()->Unload(0);
    }
    return;
  }

  Module* curr = 0;
  {
    MutexLocker lock(*modulesLock());
    curr = modules()->operator[](info->id);
    assert(curr != 0);
  }

  curr->Stop();

  curr->Uninit();
}

Module* ModuleRegistry::GetModule(long id)
{
  MutexLocker lock(*modulesLock());

  ModuleMap::const_iterator iter = modules()->find(id);
  if (iter != modules()->end())
  {
    return iter->second;
  }
  return 0;
}

Module* ModuleRegistry::GetModule(const std::string& name)
{
  MutexLocker lock(*modulesLock());

  ModuleMap::const_iterator iter = modules()->begin();
  ModuleMap::const_iterator iterEnd = modules()->end();
  for (; iter != iterEnd; ++iter)
  {
    if (iter->second->GetName() == name)
    {
      return iter->second;
    }
  }

  return 0;
}

void ModuleRegistry::GetModules(std::vector<Module*>& m)
{
  MutexLocker lock(*modulesLock());

  ModuleMap* map = modules();
  ModuleMap::const_iterator iter = map->begin();
  ModuleMap::const_iterator iterEnd = map->end();
  for (; iter != iterEnd; ++iter)
  {
    m.push_back(iter->second);
  }
}

void ModuleRegistry::GetLoadedModules(std::vector<Module*>& m)
{
  MutexLocker lock(*modulesLock());

  ModuleMap::const_iterator iter = modules()->begin();
  ModuleMap::const_iterator iterEnd = modules()->end();
  for (; iter != iterEnd; ++iter)
  {
    if (iter->second->IsLoaded())
    {
      m.push_back(iter->second);
    }
  }
}

US_END_NAMESPACE
