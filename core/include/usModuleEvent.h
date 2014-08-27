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

#ifndef USMODULEEVENT_H
#define USMODULEEVENT_H

#include <iostream>

#include "usSharedData.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif

US_BEGIN_NAMESPACE

class Module;
class ModuleEventData;

/**
 * \ingroup MicroServices
 *
 * An event from the Micro Services framework describing a module lifecycle change.
 * <p>
 * <code>ModuleEvent</code> objects are delivered to listeners connected
 * via ModuleContext::AddModuleListener() when a change
 * occurs in a modules's lifecycle. A type code is used to identify
 * the event type for future extendability.
 *
 * @see ModuleContext#AddModuleListener
 */
class US_Core_EXPORT ModuleEvent
{

  SharedDataPointer<ModuleEventData> d;

public:

  enum Type {

    /**
     * The module has been loaded.
     * <p>
     * The module's
     * \link ModuleActivator::Load(ModuleContext*) ModuleActivator Load\endlink method
     * has been executed.
     */
    LOADED,

    /**
     * The module has been unloaded.
     * <p>
     * The module's
     * \link ModuleActivator::Unload(ModuleContext*) ModuleActivator Unload\endlink method
     * has been executed.
     */
    UNLOADED,

    /**
     * The module is about to be loaded.
     * <p>
     * The module's
     * \link ModuleActivator::Load(ModuleContext*) ModuleActivator Load\endlink method
     * is about to be called.
     */
    LOADING,

    /**
     * The module is about to be unloaded.
     * <p>
     * The module's
     * \link ModuleActivator::Unload(ModuleContext*) ModuleActivator Unload\endlink method
     * is about to be called.
     */
    UNLOADING

  };

  /**
   * Creates an invalid instance.
   */
  ModuleEvent();

  ~ModuleEvent();

  /**
   * Can be used to check if this ModuleEvent instance is valid,
   * or if it has been constructed using the default constructor.
   *
   * @return <code>true</code> if this event object is valid,
   *         <code>false</code> otherwise.
   */
  bool IsNull() const;

  /**
   * Creates a module event of the specified type.
   *
   * @param type The event type.
   * @param module The module which had a lifecycle change.
   */
  ModuleEvent(Type type, Module* module);

  ModuleEvent(const ModuleEvent& other);

  ModuleEvent& operator=(const ModuleEvent& other);

  /**
   * Returns the module which had a lifecycle change.
   *
   * @return The module that had a change occur in its lifecycle.
   */
  Module* GetModule() const;

  /**
   * Returns the type of lifecyle event. The type values are:
   * <ul>
   * <li>{@link #LOADING}
   * <li>{@link #LOADED}
   * <li>{@link #UNLOADING}
   * <li>{@link #UNLOADED}
   * </ul>
   *
   * @return The type of lifecycle event.
   */
  Type GetType() const;

};

/**
 * \ingroup MicroServices
 * @{
 */
US_Core_EXPORT std::ostream& operator<<(std::ostream& os, ModuleEvent::Type eventType);
US_Core_EXPORT std::ostream& operator<<(std::ostream& os, const ModuleEvent& event);
/** @}*/

US_END_NAMESPACE

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // USMODULEEVENT_H
