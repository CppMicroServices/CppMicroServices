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

#ifndef USMODULEEVENTHOOK_H
#define USMODULEEVENTHOOK_H

#include "usServiceInterface.h"
#include "usShrinkableVector.h"

US_BEGIN_NAMESPACE

class ModuleContext;
class ModuleEvent;

/**
 * @ingroup MicroServices
 *
 * %Module Event Hook Service.
 *
 * <p>
 * Modules registering this service will be called during module lifecycle
 * (loading, loaded, unloading, and unloaded) operations.
 *
 * @remarks Implementations of this interface are required to be thread-safe.
 */
struct US_Core_EXPORT ModuleEventHook
{

  virtual ~ModuleEventHook();

  /**
   * Module event hook method. This method is called prior to module event
   * delivery when a module is loading, loaded, unloading, or unloaded.
   * This method can filter the modules which receive the event.
   * <p>
   * This method is called one and only one time for
   * each module event generated, this includes module events which are
   * generated when there are no module listeners registered.
   *
   * @param event The module event to be delivered.
   * @param contexts A list of Module Contexts for modules which have
   *        listeners to which the specified event will be delivered. The
   *        implementation of this method may remove module contexts from the
   *        list to prevent the event from being delivered to the
   *        associated modules.
   */
  virtual void Event(const ModuleEvent& event, ShrinkableVector<ModuleContext*>& contexts) = 0;
};

US_END_NAMESPACE

#endif // USMODULEEVENTHOOK_H
