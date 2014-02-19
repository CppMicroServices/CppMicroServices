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

#ifndef USMODULEHOOKS_P_H
#define USMODULEHOOKS_P_H

#include "usServiceListeners_p.h"

#include <vector>

US_BEGIN_NAMESPACE

class CoreModuleContext;
class Module;
class ModuleContext;
class ModuleEvent;

class ModuleHooks
{

private:

  CoreModuleContext* const coreCtx;

public:

  ModuleHooks(CoreModuleContext* ctx);

  Module* FilterModule(const ModuleContext* mc, Module* module) const;

  void FilterModules(const ModuleContext* mc, std::vector<Module*>& modules) const;

  void FilterModuleEventReceivers(const ModuleEvent& evt,
                                  ServiceListeners::ModuleListenerMap& moduleListeners);

};

US_END_NAMESPACE

#endif // USMODULEHOOKS_P_H
