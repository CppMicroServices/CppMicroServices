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

#include "usModuleRegistry.h"

#include "usModule.h"
#include "usModuleInfo.h"
#include "usModuleContext.h"
#include "usModuleActivator.h"
#include "usModuleInitialization.h"
#include "usCoreModuleContext_p.h"
#include "usGetModuleContext.h"
#include "usStaticInit_p.h"

#include <cassert>
#include <map>


US_BEGIN_NAMESPACE

typedef US_UNORDERED_MAP_TYPE<std::string, Module*> ModuleMap;

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
US_GLOBAL_STATIC(Mutex, modulesLock)

/**
 * Lock for protecting the register count
 */
US_GLOBAL_STATIC(Mutex, countLock)

void ModuleRegistry::Register(ModuleInfo* info)
{
  static long regCount = 0;
  if (info->id > 0)
  {
    // The module was already registered
    Module* module = 0;
    {
      MutexLock lock(*modulesLock());
      module = modules()->operator[](info->name);
      assert(module != 0);
    }
    module->Start();
  }
  else
  {
    Module* module = 0;
    // check if the module is reloaded
    {
      MutexLock lock(*modulesLock());
      ModuleMap* map = modules();
      for (ModuleMap::const_iterator i = map->begin();
           i != map->end(); ++i)
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
      countLock()->Lock();
      info->id = ++regCount;
      assert(info->id == 1 ? info->name == "CppMicroServices" : true);
      countLock()->Unlock();

      module->Init(coreModuleContext(), info);

      MutexLock lock(*modulesLock());
      ModuleMap* map = modules();
      map->insert(std::make_pair(info->name, module));
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
  if (info->id > 1)
  {
    Module* curr = 0;
    {
      MutexLock lock(*modulesLock());
      curr = modules()->operator[](info->name);
      assert(curr != 0);
    }
    curr->Stop();
  }
}

Module* ModuleRegistry::GetModule(long id)
{
  MutexLock lock(*modulesLock());

  ModuleMap::const_iterator iter = modules()->begin();
  ModuleMap::const_iterator iterEnd = modules()->end();
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
  MutexLock lock(*modulesLock());

  ModuleMap::const_iterator iter = modules()->find(name);
  if (iter != modules()->end())
  {
    return iter->second;
  }
  return 0;
}

std::vector<Module*> ModuleRegistry::GetModules()
{
  MutexLock lock(*modulesLock());

  std::vector<Module*> result;
  ModuleMap* map = modules();
  ModuleMap::const_iterator iter = map->begin();
  ModuleMap::const_iterator iterEnd = map->end();
  for (; iter != iterEnd; ++iter)
  {
    result.push_back(iter->second);
  }
  return result;
}

std::vector<Module*> ModuleRegistry::GetLoadedModules()
{
  MutexLock lock(*modulesLock());

  std::vector<Module*> result;
  ModuleMap::const_iterator iter = modules()->begin();
  ModuleMap::const_iterator iterEnd = modules()->end();
  for (; iter != iterEnd; ++iter)
  {
    if (iter->second->IsLoaded())
    {
      result.push_back(iter->second);
    }
  }
  return result;
}

// Control the static initialization order for several core objects
struct StaticInitializationOrder
{
  StaticInitializationOrder()
  {
    ModuleSettings::GetLogLevel();
    modulesLock();
    countLock();
    modules();
    coreModuleContext();
  }
};

static StaticInitializationOrder _staticInitializationOrder;

US_END_NAMESPACE

// We initialize the CppMicroService module after making sure
// that all other global statics have been initialized above
US_INITIALIZE_MODULE
