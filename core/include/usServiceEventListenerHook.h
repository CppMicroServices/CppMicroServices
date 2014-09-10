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

#ifndef USSERVICEEVENTLISTENERHOOK_H
#define USSERVICEEVENTLISTENERHOOK_H

#include "usServiceInterface.h"
#include "usServiceListenerHook.h"
#include "usShrinkableVector.h"
#include "usShrinkableMap.h"

US_BEGIN_NAMESPACE

class ModuleContext;
class ServiceEvent;

/**
 * @ingroup MicroServices
 *
 * Service Event Listener Hook Service.
 *
 * <p>
 * Modules registering this service will be called during service
 * (register, modify, and unregister service) operations.
 *
 * @remarks Implementations of this interface are required to be thread-safe.
 */
struct US_Core_EXPORT ServiceEventListenerHook
{
  typedef ShrinkableMap<ModuleContext*, ShrinkableVector<ServiceListenerHook::ListenerInfo> > ShrinkableMapType;

  virtual ~ServiceEventListenerHook();

  /**
   * Event listener hook method. This method is called prior to service event
   * delivery when a publishing module registers, modifies or unregisters a
   * service. This method can filter the listeners which receive the event.
   *
   * @param event The service event to be delivered.
   * @param listeners A map of Module Contexts to a list of Listener
   *        Infos for the module's listeners to which the specified event will
   *        be delivered. The implementation of this method may remove module
   *        contexts from the map and listener infos from the list
   *        values to prevent the event from being delivered to the associated
   *        listeners.
   */
  virtual void Event(const ServiceEvent& event, ShrinkableMapType& listeners) = 0;
};

US_END_NAMESPACE

#endif // USSERVICEEVENTLISTENERHOOK_H
