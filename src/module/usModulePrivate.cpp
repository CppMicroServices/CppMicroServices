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
#include "usCoreModuleContext_p.h"
#include "usServiceRegistration.h"
#include "usServiceReferencePrivate.h"

US_BEGIN_NAMESPACE

AtomicInt ModulePrivate::idCounter;

ModulePrivate::ModulePrivate(Module* qq, CoreModuleContext* coreCtx,
                             ModuleInfo* info)
  : coreCtx(coreCtx), info(*info), moduleContext(0), moduleActivator(0), q(qq)
{

  std::stringstream propId;
  propId << this->info.id;
  properties[Module::PROP_ID()] = propId.str();

  std::stringstream propModuleDepends;
  std::stringstream propLibDepends;
  std::stringstream propPackageDepends;

  int counter = 0;
  int counter2 = 0;
  std::stringstream ss(this->info.moduleDeps);
  while (ss)
  {
    std::string moduleDep;
    ss >> moduleDep;
    if (!moduleDep.empty())
    {
      Module* dep = ModuleRegistry::GetModule(moduleDep);
      if (dep)
      {
        requiresIds.push_back(dep->GetModuleId());
        if (counter > 0) propModuleDepends << ", ";
        propModuleDepends << moduleDep;
        ++counter;
      }
      else
      {
        requiresLibs.push_back(moduleDep);
        if (counter2 > 0) propLibDepends << ", ";
        propLibDepends << moduleDep;
        ++counter2;
      }
    }
  }

  properties[Module::PROP_MODULE_DEPENDS()] = propModuleDepends.str();
  properties[Module::PROP_LIB_DEPENDS()] = propLibDepends.str();

  if (!this->info.version.empty())
  {
    try
    {
      version = ModuleVersion(this->info.version);
      properties[Module::PROP_VERSION()] = this->info.version;
    }
    catch (const std::exception& e)
    {
      throw std::invalid_argument(std::string("US module does not specify a valid version identifier. Got exception: ") + e.what());
    }
  }

  properties[Module::PROP_LOCATION()] = this->info.location;
  properties[Module::PROP_NAME()] = this->info.name;
}

ModulePrivate::~ModulePrivate()
{
  delete moduleContext;
}

void ModulePrivate::RemoveModuleResources()
{
  coreCtx->listeners.RemoveAllListeners(moduleContext);
 
  std::list<ServiceRegistration> srs;
  coreCtx->services.GetRegisteredByModule(this, srs);
  for (std::list<ServiceRegistration>::iterator i = srs.begin();
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
  for (std::list<ServiceRegistration>::const_iterator i = srs.begin();
       i != srs.end(); ++i)
  {
    i->GetReference().d->UngetService(q, false);
  }
}

US_END_NAMESPACE

