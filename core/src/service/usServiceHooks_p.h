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

#ifndef USSERVICEHOOKS_P_H
#define USSERVICEHOOKS_P_H

#include "usServiceTracker.h"
#include "usServiceListeners_p.h"

US_BEGIN_NAMESPACE

struct ServiceListenerHook;

class ServiceHooks : private MultiThreaded<>, private ServiceTrackerCustomizer<ServiceListenerHook>
{

private:

  CoreModuleContext* coreCtx;
  ServiceTracker<ServiceListenerHook>* listenerHookTracker;

  bool bOpen;

  virtual TrackedType AddingService(const ServiceReferenceType& reference);
  virtual void ModifiedService(const ServiceReferenceType& reference, TrackedType service);
  virtual void RemovedService(const ServiceReferenceType& reference, TrackedType service);

public:

  ServiceHooks(CoreModuleContext* coreCtx);
  ~ServiceHooks();

  void Open();

  void Close();

  bool IsOpen() const;

  void FilterServiceReferences(ModuleContext* mc, const std::string& service,
                               const std::string& filter, std::vector<ServiceReferenceBase>& refs);

  void FilterServiceEventReceivers(const ServiceEvent& evt,
                                   ServiceListeners::ServiceListenerEntries& receivers);

  void HandleServiceListenerReg(const ServiceListenerEntry& sle);

  void HandleServiceListenerUnreg(const ServiceListenerEntry& sle);

  void HandleServiceListenerUnreg(const std::vector<ServiceListenerEntry>& set);

};

US_END_NAMESPACE

#endif // USSERVICEHOOKS_P_H
