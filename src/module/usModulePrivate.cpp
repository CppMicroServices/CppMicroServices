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

#include "usModulePrivate.h"

#include "usModule.h"
#include "usModuleContext.h"
#include "usModuleActivator.h"
#include "usModuleUtils_p.h"
#include "usModuleSettings.h"
#include "usModuleResource.h"
#include "usModuleResourceStream.h"
#include "usCoreModuleContext_p.h"
#include "usServiceRegistration.h"
#include "usServiceReferenceBasePrivate.h"

#include <algorithm>
#include <iterator>
#include <cassert>
#include <cstring>

US_BEGIN_NAMESPACE

AtomicInt ModulePrivate::idCounter;

ModulePrivate::ModulePrivate(Module* qq, CoreModuleContext* coreCtx,
                             ModuleInfo* info)
  : coreCtx(coreCtx)
  , info(*info)
  , moduleContext(0)
  , moduleActivator(0)
  , q(qq)
{
  // Parse the statically imported module library names
  typedef const char*(*GetImportedModulesFunc)(void);

  std::string getImportedModulesSymbol("_us_get_imported_modules_for_");
  getImportedModulesSymbol += this->info.libName;

  std::string location = this->info.location;
  if (this->info.libName.empty())
  {
    /* make sure we retrieve symbols from the executable, if "libName" is empty */
    location.clear();
  }

  GetImportedModulesFunc getImportedModulesFunc = NULL;
  void* getImportedModulesSym = ModuleUtils::GetSymbol(location, getImportedModulesSymbol.c_str());
  std::memcpy(&getImportedModulesFunc, &getImportedModulesSym, sizeof(void*));
  if (getImportedModulesFunc != NULL)
  {
    std::string importedStaticModuleLibNames = getImportedModulesFunc();

    std::istringstream iss(importedStaticModuleLibNames);
    std::copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(),
              std::back_inserter<std::vector<std::string> >(this->staticModuleLibNames));
  }

  InitializeResources(location);

  // Check if the module provides a manifest.json file and if yes, parse it.
  ModuleResource manifestRes;
  std::map<std::string, ModuleResourceTree*>::iterator resourceTreeIter = mapLibNameToResourceTrees.find(this->info.libName);
  if (resourceTreeIter != mapLibNameToResourceTrees.end() && resourceTreeIter->second->IsValid())
  {
    manifestRes = ModuleResource("/manifest.json", resourceTreeIter->second, resourceTreePtrs);
    if (manifestRes)
    {
      ModuleResourceStream manifestStream(manifestRes);
      try
      {
        moduleManifest.Parse(manifestStream);
      }
      catch (const std::exception& e)
      {
        US_ERROR << "Parsing of manifest.json for module " << info->location << " failed: " << e.what();
      }
    }
  }

  // Check if we got version information and validate the version identifier
  if (moduleManifest.Contains(Module::PROP_VERSION()))
  {
    Any versionAny = moduleManifest.GetValue(Module::PROP_VERSION());
    std::string errMsg;
    if (versionAny.Type() != typeid(std::string))
    {
      errMsg = std::string("The version identifier must be a string");
    }
    try
    {
      version = ModuleVersion(versionAny.ToString());
    }
    catch (const std::exception& e)
    {
      errMsg = std::string("The version identifier is invalid: ") + e.what();
    }

    if (!errMsg.empty())
    {
      throw std::invalid_argument(std::string("The Json value for ") + Module::PROP_VERSION() + " for module " +
                                  info->location + " is not valid: " + errMsg);
    }
  }

  std::stringstream propId;
  propId << this->info.id;
  moduleManifest.SetValue(Module::PROP_ID(), propId.str());
  moduleManifest.SetValue(Module::PROP_LOCATION(), this->info.location);
  moduleManifest.SetValue(Module::PROP_NAME(), this->info.name);

  if (moduleManifest.Contains(Module::PROP_AUTOLOAD_DIR()))
  {
    this->info.autoLoadDir = moduleManifest.GetValue(Module::PROP_AUTOLOAD_DIR()).ToString();
  }
  else
  {
    // default to the library name or a special name for executables
    if (!this->info.libName.empty())
    {
      this->info.autoLoadDir = this->info.libName;
      moduleManifest.SetValue(Module::PROP_AUTOLOAD_DIR(), Any(this->info.autoLoadDir));
    }
    else
    {
      this->info.autoLoadDir = "main";
      moduleManifest.SetValue(Module::PROP_AUTOLOAD_DIR(), Any(this->info.autoLoadDir));
    }
  }

  // comput the module storage path
#ifdef US_PLATFORM_WINDOWS
    static const char separator = '\\';
#else
    static const char separator = '/';
#endif

  std::string baseStoragePath = ModuleSettings::GetStoragePath();
  if (!baseStoragePath.empty())
  {
    char buf[50];
    sprintf(buf, "%ld", this->info.id);
    storagePath = baseStoragePath + separator + buf + "_" + this->info.libName + separator;
  }
}

ModulePrivate::~ModulePrivate()
{
  delete moduleContext;

  for (std::size_t i = 0; i < this->resourceTreePtrs.size(); ++i)
  {
    delete resourceTreePtrs[i];
  }
}

void ModulePrivate::RemoveModuleResources()
{
  coreCtx->listeners.RemoveAllListeners(moduleContext);

  std::vector<ServiceRegistrationBase> srs;
  coreCtx->services.GetRegisteredByModule(this, srs);
  for (std::vector<ServiceRegistrationBase>::iterator i = srs.begin();
       i != srs.end(); ++i)
  {
    try
    {
      i->Unregister();
    }
    catch (const std::logic_error& /*ignore*/)
    {
      // Someone has unregistered the service after stop completed.
      // This should not occur, but we don't want get stuck in
      // an illegal state so we catch it.
    }
  }

  srs.clear();
  coreCtx->services.GetUsedByModule(q, srs);
  for (std::vector<ServiceRegistrationBase>::const_iterator i = srs.begin();
       i != srs.end(); ++i)
  {
    i->GetReference(std::string()).d->UngetService(q, false);
  }

  for (std::size_t i = 0; i < resourceTreePtrs.size(); ++i)
  {
    resourceTreePtrs[i]->Invalidate();
  }
}

void ModulePrivate::StartStaticModules()
{
  std::string location = this->info.location;
  if (this->info.libName.empty())
  {
    /* make sure we retrieve symbols from the executable, if "libName" is empty */
    location.clear();
  }

  for (std::vector<std::string>::iterator i = staticModuleLibNames.begin();
       i != staticModuleLibNames.end(); ++i)
  {
    std::string staticActivatorSymbol = "_us_module_activator_instance_";
    staticActivatorSymbol += *i;
    ModuleInfo::ModuleActivatorHook staticActivator = NULL;
    void* staticActivatorSym = ModuleUtils::GetSymbol(location, staticActivatorSymbol.c_str());
    std::memcpy(&staticActivator, &staticActivatorSym, sizeof(void*));
    if (staticActivator)
    {
      US_DEBUG << "Loading static activator " << *i;
      staticActivators.push_back(staticActivator);
      staticActivator()->Load(moduleContext);
    }
    else
    {
      US_DEBUG << "Could not find an activator for the static module " << (*i)
               << ". It propably does not provide an activator on purpose.\n Or you either "
                  "forgot a US_IMPORT_MODULE macro call in " << info.libName
               << " or to link " << (*i) << " to " << info.libName << ".";
    }
  }

}

void ModulePrivate::StopStaticModules()
{
  for (std::list<ModuleInfo::ModuleActivatorHook>::iterator i = staticActivators.begin();
       i != staticActivators.end(); ++i)
  {
    (*i)()->Unload(moduleContext);
  }
}

void ModulePrivate::InitializeResources(const std::string& location)
{
  // Get the resource data from static modules and this module
  std::vector<std::string> moduleLibNames;
  moduleLibNames.push_back(this->info.libName);
  moduleLibNames.insert(moduleLibNames.end(),
                        this->staticModuleLibNames.begin(), this->staticModuleLibNames.end());

  std::string initResourcesSymbolPrefix = "_us_init_resources_";
  for (std::size_t i = 0; i < moduleLibNames.size(); ++i)
  {
    std::string initResourcesSymbol = initResourcesSymbolPrefix + moduleLibNames[i];
    ModuleInfo::InitResourcesHook initResourcesFunc = NULL;
    void* initResourcesSym = ModuleUtils::GetSymbol(location, initResourcesSymbol.c_str());
    std::memcpy(&initResourcesFunc, &initResourcesSym, sizeof(void*));
    if (initResourcesFunc)
    {
      initResourcesFunc(&this->info);
    }
  }

  // Initialize this modules resource trees
  assert(this->info.resourceData.size() == this->info.resourceNames.size());
  assert(this->info.resourceNames.size() == this->info.resourceTree.size());
  for (std::size_t i = 0; i < this->info.resourceData.size(); ++i)
  {
    resourceTreePtrs.push_back(new ModuleResourceTree(this->info.resourceTree[i],
                                                      this->info.resourceNames[i],
                                                      this->info.resourceData[i]));
    mapLibNameToResourceTrees[moduleLibNames[i]] = resourceTreePtrs.back();
  }
}

US_END_NAMESPACE
