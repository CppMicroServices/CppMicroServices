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


#include "usModule.h"

#include "usModuleContext.h"
#include "usModuleActivator.h"
#include "usModulePrivate.h"
#include "usModuleResource.h"
#include "usModuleSettings.h"
#include "usCoreModuleContext_p.h"

#include "usCoreConfig.h"

US_BEGIN_NAMESPACE

const std::string& Module::PROP_ID()
{
  static const std::string s("module.id");
  return s;
}
const std::string& Module::PROP_NAME()
{
  static const std::string s("module.name");
  return s;
}
const std::string& Module::PROP_LOCATION()
{
  static const std::string s("module.location");
  return s;
}
const std::string& Module::PROP_VERSION()
{
  static const std::string s("module.version");
  return s;
}

const std::string&Module::PROP_VENDOR()
{
  static const std::string s("module.vendor");
  return s;
}

const std::string&Module::PROP_DESCRIPTION()
{
  static const std::string s("module.description");
  return s;
}

const std::string&Module::PROP_AUTOLOAD_DIR()
{
  static const std::string s("module.autoload_dir");
  return s;
}

const std::string&Module::PROP_AUTOLOADED_MODULES()
{
  static const std::string s("module.autoloaded_modules");
  return s;
}

Module::Module()
: d(0)
{

}

Module::~Module()
{
  delete d;
}

void Module::Init(CoreModuleContext* coreCtx,
                  ModuleInfo* info)
{
  ModulePrivate* mp = new ModulePrivate(this, coreCtx, info);
  std::swap(mp, d);
  delete mp;
}

void Module::Uninit()
{
  if (d->moduleContext != NULL)
  {
    //d->coreCtx->listeners.HooksModuleStopped(d->moduleContext);
    d->RemoveModuleResources();
    delete d->moduleContext;
    d->moduleContext = 0;
    d->coreCtx->listeners.ModuleChanged(ModuleEvent(ModuleEvent::UNLOADED, this));

    d->moduleActivator = 0;
  }
}

bool Module::IsLoaded() const
{
  return d->moduleContext != 0;
}

void Module::Start()
{

  if (d->moduleContext)
  {
    US_WARN << "Module " << d->info.name << " already started.";
    return;
  }

  d->moduleContext = new ModuleContext(this->d);

  typedef ModuleActivator*(*ModuleActivatorHook)(void);
  ModuleActivatorHook activatorHook = NULL;

  std::string activator_func = "_us_module_activator_instance_" + d->info.name;
  void* activatorHookSym = ModuleUtils::GetSymbol(d->info, activator_func.c_str());
  std::memcpy(&activatorHook, &activatorHookSym, sizeof(void*));

  d->coreCtx->listeners.ModuleChanged(ModuleEvent(ModuleEvent::LOADING, this));
  // try to get a ModuleActivator instance

  if (activatorHook)
  {
    try
    {
      d->moduleActivator = activatorHook();
    }
    catch (...)
    {
      US_ERROR << "Creating the module activator of " << d->info.name << " failed";
      throw;
    }

    // This method should be "noexcept" and by not catching exceptions
    // here we semantically treat it that way since any exception during
    // static initialization will either terminate the program or cause
    // the dynamic loader to report an error.
    d->moduleActivator->Load(d->moduleContext);
  }

#ifdef US_ENABLE_AUTOLOADING_SUPPORT
  if (ModuleSettings::IsAutoLoadingEnabled())
  {
    const std::vector<std::string> loadedPaths = AutoLoadModules(d->info);
    if (!loadedPaths.empty())
    {
      d->moduleManifest.SetValue(PROP_AUTOLOADED_MODULES(), Any(loadedPaths));
    }
  }
#endif

  d->coreCtx->listeners.ModuleChanged(ModuleEvent(ModuleEvent::LOADED, this));
}

void Module::Stop()
{
  if (d->moduleContext == 0)
  {
    US_WARN << "Module " << d->info.name << " already stopped.";
    return;
  }

  try
  {
    d->coreCtx->listeners.ModuleChanged(ModuleEvent(ModuleEvent::UNLOADING, this));

    if (d->moduleActivator)
    {
      d->moduleActivator->Unload(d->moduleContext);
    }
  }
  catch (...)
  {
    US_WARN << "Calling the module activator Unload() method of " << d->info.name << " failed!";

    try
    {
      this->Uninit();
    }
    catch (...) {}

    throw;
  }

  this->Uninit();
}

ModuleContext* Module::GetModuleContext() const
{
  return d->moduleContext;
}

long Module::GetModuleId() const
{
  return d->info.id;
}

std::string Module::GetLocation() const
{
  return d->info.location;
}

std::string Module::GetName() const
{
  return d->info.name;
}

ModuleVersion Module::GetVersion() const
{
  return d->version;
}

Any Module::GetProperty(const std::string& key) const
{
  return d->moduleManifest.GetValue(key);
}

std::vector<std::string> Module::GetPropertyKeys() const
{
  return d->moduleManifest.GetKeys();
}

std::vector<ServiceReferenceU> Module::GetRegisteredServices() const
{
  std::vector<ServiceRegistrationBase> sr;
  std::vector<ServiceReferenceU> res;
  d->coreCtx->services.GetRegisteredByModule(d, sr);
  for (std::vector<ServiceRegistrationBase>::const_iterator i = sr.begin();
       i != sr.end(); ++i)
  {
    res.push_back(i->GetReference());
  }
  return res;
}

std::vector<ServiceReferenceU> Module::GetServicesInUse() const
{
  std::vector<ServiceRegistrationBase> sr;
  std::vector<ServiceReferenceU> res;
  d->coreCtx->services.GetUsedByModule(const_cast<Module*>(this), sr);
  for (std::vector<ServiceRegistrationBase>::const_iterator i = sr.begin();
       i != sr.end(); ++i)
  {
    res.push_back(i->GetReference());
  }
  return res;
}

ModuleResource Module::GetResource(const std::string& path) const
{
  if (!d->resourceContainer.IsValid())
  {
    return ModuleResource();
  }
  ModuleResource result(path, d->resourceContainer);
  if (result) return result;
  return ModuleResource();
}

std::vector<ModuleResource> Module::FindResources(const std::string& path, const std::string& filePattern,
                                                  bool recurse) const
{
  std::vector<ModuleResource> result;
  if (!d->resourceContainer.IsValid())
  {
    return result;
  }

  std::string normalizedPath = path;
  // add a leading and trailing slash
  if (normalizedPath.empty()) normalizedPath.push_back('/');
  if (*normalizedPath.begin() != '/') normalizedPath = '/' + normalizedPath;
  if (*normalizedPath.rbegin() != '/') normalizedPath.push_back('/');
  d->resourceContainer.FindNodes(d->info.name + normalizedPath,
                                 filePattern.empty() ? "*" : filePattern,
                                 recurse, result);
  return result;
}

US_END_NAMESPACE

US_USE_NAMESPACE

std::ostream& operator<<(std::ostream& os, const Module& module)
{
  os << "Module[" << "id=" << module.GetModuleId() <<
        ", loc=" << module.GetLocation() <<
        ", name=" << module.GetName() << "]";
  return os;
}

std::ostream& operator<<(std::ostream& os, Module const * module)
{
  return operator<<(os, *module);
}
