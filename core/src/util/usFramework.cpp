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

#include "usFramework.h"

#include "usCoreModuleContext_p.h"
#include "usModuleInfo.h"
#include "usModuleSettings.h"
#include "usModuleUtils_p.h"

US_BEGIN_NAMESPACE

// TODO: how do we bootstrap the framework for installation
// into the bundle registry? Do we hard code the Framework name or not?
const std::string frameworkName("CppMicroServices");

Framework::Framework(void) : 
    coreModuleContext(new CoreModuleContext()),
    initialized(false)
{

}

Framework::Framework(std::map<std::string, std::string>& configuration) :
    coreModuleContext(new CoreModuleContext()),
    launchProperties(configuration),
    initialized(false)
{
  coreModuleContext->launchProperties = configuration;
}

Framework::~Framework(void)
{
  if(coreModuleContext)
  {
    delete coreModuleContext;
  }

  initialized = false;
}

void Framework::init(void) 
{
  ModuleInfo* moduleInfo = new ModuleInfo(frameworkName);

  void(Framework::*initFncPtr)(void) = &Framework::init;
  void* frameworkInit = NULL;
  std::memcpy(&frameworkInit, &initFncPtr, sizeof(void*));
  moduleInfo->location = ModuleUtils::GetLibraryPath(frameworkInit);

  Module* systemBundle = coreModuleContext->bundleRegistry.Register(moduleInfo);
  if(systemBundle)
  {
    // TODO: correctly construct the Module base class. For now, temporarily allow the bundle context to be used.
    this->d = systemBundle->d;
    systemBundle->Start();
    // TODO: make thread-safe
    initialized = true;
  }
}

void Framework::Start() 
{ 
  if(!initialized)
  {
    init();
  }
}

void Framework::Uninstall() 
{
  /* TODO: throw a BundleException class, per OSGi standard */ 
  throw std::runtime_error("Cannot uninstall a system bundle."); 
}

std::string Framework::GetLocation() const
{
  // per OSGi Core release 6, sectiopn 4.6 - 
  //  The system bundle getLocation method returns the string: "System Bundle",   //  as defined in the Constants interface.
  // TODO: do we need to implement an equivalent Constants interface?
  return std::string("System Bundle");
}

void Framework::SetAutoLoadingEnabled(bool enable)
{
  coreModuleContext->settings.SetAutoLoadingEnabled(enable);
}

US_END_NAMESPACE