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

#ifndef CPPMICROSERVICES_SERVICELISTENERHOOK_P_H
#define CPPMICROSERVICES_SERVICELISTENERHOOK_P_H

#include "cppmicroservices/ServiceListenerHook.h"
#include "cppmicroservices/SharedData.h"

#include "ServiceListenerEntry_p.h"

namespace cppmicroservices {

class ServiceListenerHook::ListenerInfoData : public SharedData
{
public:
  ListenerInfoData(const std::shared_ptr<BundleContextPrivate>& context, const ServiceListener& l,
                   void* data, const std::string& filter);

  virtual ~ListenerInfoData();

  std::shared_ptr<BundleContextPrivate> const context;
  ServiceListener listener;
  void* data;
  std::string filter;
  bool bRemoved;
};

}

#endif // CPPMICROSERVICES_SERVICELISTENERHOOK_P_H
