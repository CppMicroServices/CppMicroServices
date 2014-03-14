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

#ifndef USSERVICELISTENERHOOK_P_H
#define USSERVICELISTENERHOOK_P_H

#include "usServiceListenerHook.h"
#include "usServiceListenerEntry_p.h"
#include "usSharedData.h"

US_BEGIN_NAMESPACE

class ServiceListenerHook::ListenerInfoData : public SharedData
{
public:
  ListenerInfoData(ModuleContext* mc, const ServiceListenerEntry::ServiceListener& l,
                   void* data, const std::string& filter);

  virtual ~ListenerInfoData();

  ModuleContext* const mc;
  ServiceListenerEntry::ServiceListener listener;
  void* data;
  std::string filter;
  bool bRemoved;
};

US_END_NAMESPACE

#endif // USSERVICELISTENERHOOK_P_H
