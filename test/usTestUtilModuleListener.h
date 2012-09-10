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

#ifndef USTESTUTILMODULELISTENER_H
#define USTESTUTILMODULELISTENER_H

#include "usConfig.h"

#include "usModuleEvent.h"
#include "usServiceEvent.h"

US_BEGIN_NAMESPACE

class ModuleContext;

class TestModuleListener {

public:

  TestModuleListener(ModuleContext* mc);

  void ModuleChanged(const ModuleEvent event);

  void ServiceChanged(const ServiceEvent event);

  ModuleEvent GetModuleEvent() const;

  ServiceEvent GetServiceEvent() const;

  bool CheckListenerEvents(
      bool pexp, ModuleEvent::Type ptype,
      bool sexp, ServiceEvent::Type stype,
      Module* moduleX, ServiceReference* servX);

  bool CheckListenerEvents(const std::vector<ModuleEvent>& pEvts);

  bool CheckListenerEvents(const std::vector<ServiceEvent>& seEvts);

  bool CheckListenerEvents(const std::vector<ModuleEvent>& pEvts,
                           const std::vector<ServiceEvent>& seEvts);

private:

  ModuleContext* const mc;

  std::vector<ServiceEvent> serviceEvents;
  std::vector<ModuleEvent> moduleEvents;
};

US_END_NAMESPACE

#endif // USTESTUTILMODULELISTENER_H
