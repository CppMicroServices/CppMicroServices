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

#include "usTestUtilModuleListener.h"

#include "usUtils_p.h"

US_BEGIN_NAMESPACE

TestModuleListener::TestModuleListener(ModuleContext* mc)
  : mc(mc), serviceEvents(), moduleEvents()
{}

void TestModuleListener::ModuleChanged(const ModuleEvent event)
{
  moduleEvents.push_back(event);
  US_DEBUG << "ModuleEvent:" << event;
}

void TestModuleListener::ServiceChanged(const ServiceEvent event)
{
  serviceEvents.push_back(event);
  US_DEBUG << "ServiceEvent:" << event;
}

ModuleEvent TestModuleListener::GetModuleEvent() const
{
  if (moduleEvents.empty())
  {
    return ModuleEvent();
  }
  return moduleEvents.back();
}

ServiceEvent TestModuleListener::GetServiceEvent() const
{
  if (serviceEvents.empty())
  {
    return ServiceEvent();
  }
  return serviceEvents.back();
}

bool TestModuleListener::CheckListenerEvents(
    bool pexp, ModuleEvent::Type ptype,
    bool sexp, ServiceEvent::Type stype,
    Module* moduleX, ServiceReference* servX)
{
  std::vector<ModuleEvent> pEvts;
  std::vector<ServiceEvent> seEvts;

  if (pexp) pEvts.push_back(ModuleEvent(ptype, moduleX));
  if (sexp) seEvts.push_back(ServiceEvent(stype, *servX));

  return CheckListenerEvents(pEvts, seEvts);
}

bool TestModuleListener::CheckListenerEvents(const std::vector<ModuleEvent>& pEvts)
{
  bool listenState = true; // assume everything will work

  if (pEvts.size() != moduleEvents.size())
  {
    listenState = false;
    US_DEBUG << "*** Module event mismatch: expected "
             << pEvts.size() << " event(s), found "
             << moduleEvents.size() << " event(s).";

    const std::size_t max = pEvts.size() > moduleEvents.size() ? pEvts.size() : moduleEvents.size();
    for (std::size_t i = 0; i < max; ++i)
    {
      const ModuleEvent& pE = i < pEvts.size() ? pEvts[i] : ModuleEvent();
      const ModuleEvent& pR = i < moduleEvents.size() ? moduleEvents[i] : ModuleEvent();
      US_DEBUG << "    " << pE << " - " << pR;
    }
  }
  else
  {
    for (std::size_t i = 0; i < pEvts.size(); ++i)
    {
      const ModuleEvent& pE = pEvts[i];
      const ModuleEvent& pR = moduleEvents[i];
      if (pE.GetType() != pR.GetType()
          || pE.GetModule() != pR.GetModule())
      {
        listenState = false;
        US_DEBUG << "*** Wrong module event: " << pR << " expected " << pE;
      }
    }
  }

  moduleEvents.clear();
  return listenState;
}

bool TestModuleListener::CheckListenerEvents(const std::vector<ServiceEvent>& seEvts)
{
  bool listenState = true; // assume everything will work

  if (seEvts.size() != serviceEvents.size())
  {
    listenState = false;
    US_DEBUG << "*** Service event mismatch: expected "
             << seEvts.size() << " event(s), found "
             << serviceEvents.size() << " event(s).";

    const std::size_t max = seEvts.size() > serviceEvents.size() ? seEvts.size() : serviceEvents.size();
    for (std::size_t i = 0; i < max; ++i)
    {
      const ServiceEvent& seE = i < seEvts.size() ? seEvts[i] : ServiceEvent();
      const ServiceEvent& seR = i < serviceEvents.size() ? serviceEvents[i] : ServiceEvent();
      US_DEBUG << "    " << seE << " - " << seR;
    }
  }
  else
  {
    for (std::size_t i = 0; i < seEvts.size(); ++i)
    {
      const ServiceEvent& seE = seEvts[i];
      const ServiceEvent& seR = serviceEvents[i];
      if (seE.GetType() != seR.GetType()
          || (!(seE.GetServiceReference() == seR.GetServiceReference())))
      {
        listenState = false;
        US_DEBUG << "*** Wrong service event: " << seR << " expected " << seE;
      }
    }
  }

  serviceEvents.clear();
  return listenState;
}

bool TestModuleListener::CheckListenerEvents(
    const std::vector<ModuleEvent>& pEvts,
    const std::vector<ServiceEvent>& seEvts)
{
  if (!CheckListenerEvents(pEvts)) return false;
  return CheckListenerEvents(seEvts);
}

US_END_NAMESPACE
