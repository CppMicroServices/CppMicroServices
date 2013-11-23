/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef USSERVICELISTENERHOOK_P_H
#define USSERVICELISTENERHOOK_P_H

#include <usConfig.h>

#include "usServiceListenerHook.h"
#include "usServiceListenerEntry_p.h"
#include "usSharedData.h"

US_BEGIN_NAMESPACE

class ServiceListenerHook::ListenerInfoData : public SharedData
{
public:
  ListenerInfoData(ModuleContext* mc, const ServiceListenerEntry::ServiceListener& l,
                   void* data, const std::string& filter);

  ModuleContext* const mc;
  ServiceListenerEntry::ServiceListener listener;
  void* data;
  std::string filter;
  bool bRemoved;
};

US_END_NAMESPACE

#endif // USSERVICELISTENERHOOK_P_H
