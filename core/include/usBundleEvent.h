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

#ifndef USBUNDLEEVENT_H
#define USBUNDLEEVENT_H

#include <iostream>
#include <memory>

#include "usCoreExport.h"
#include "usSharedData.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace us {

class Bundle;
class BundleEventData;

/**
 * \ingroup MicroServices
 *
 * An event from the Micro Services framework describing a bundle lifecycle change.
 * <p>
 * <code>BundleEvent</code> objects are delivered to listeners connected
 * via BundleContext::AddBundleListener() when a change
 * occurs in a bundles's lifecycle. A type code is used to identify
 * the event type for future extendability.
 *
 * @see BundleContext#AddBundleListener
 */
class US_Core_EXPORT BundleEvent
{

  SharedDataPointer<BundleEventData> d;

public:

  enum Type {

    /**
     * The bundle has been started.
     * <p>
     * The bundle's
     * \link BundleActivator::Start(BundleContext*) BundleActivator Start\endlink method
     * has been executed.
     */
    STARTED,

    /**
     * The bundle has been stopped.
     * <p>
     * The bundle's
     * \link BundleActivator::Stop(BundleContext*) BundleActivator Stop\endlink method
     * has been executed.
     */
    STOPPED,

    /**
     * The bundle is about to be started.
     * <p>
     * The bundle's
     * \link BundleActivator::Start(BundleContext*) BundleActivator Start\endlink method
     * is about to be called.
     */
    STARTING,

    /**
     * The bundle is about to be stopped.
     * <p>
     * The bundle's
     * \link BundleActivator::Stop(BundleContext*) BundleActivator Stop\endlink method
     * is about to be called.
     */
    STOPPING,

    /**
     * The bundle has been installed.
     * <p>
     * The bundle has been installed by the Framework.
     */
    INSTALLED,

    /**
     * The bundle has been uninstalled.
     * <p>
     * The bundle has been removed from the Framework.
     */
    UNINSTALLED,

    /**
     * The bundle has been resolved.
     *
     * @see Bundle#RESOLVED
     */
    RESOLVED,

    /**
     * The bundle has been unresolved.
     *
     * @see Bundle#INSTALLED
     */
    UNRESOLVED,

    /**
     * The bundle will be lazily activated.
     * <p>
     * The bundle has a {@link Constants#ACTIVATION_LAZY lazy activation policy}
     * and is waiting to be activated. It is now in the {@link Bundle#STARTING
     * STARTING} state and has a valid {@code BundleContext}. This event is only
     * delivered to {@link SynchronousBundleListener}s. It is not delivered to
     * {@code BundleListener}s.
     */
    LAZY_ACTIVATION

  };

  /**
   * Creates an invalid instance.
   */
  BundleEvent();

  ~BundleEvent();

  /**
   * Can be used to check if this BundleEvent instance is valid,
   * or if it has been constructed using the default constructor.
   *
   * @return <code>true</code> if this event object is valid,
   *         <code>false</code> otherwise.
   */
  bool IsNull() const;

  /**
   * Creates a bundle event of the specified type.
   *
   * @param type The event type.
   * @param bundle The bundle which had a lifecycle change. This bundle is
   *        used as the origin of the event.
   */
  BundleEvent(Type type, const Bundle& bundle);

  /**
   * Creates a bundle event of the specified type.
   *
   * @param type The event type.
   * @param bundle The bundle which had a lifecycle change.
   * @param origin The bundle which is the origin of the event. For the event
   *        type {@link #INSTALLED}, this is the bundle whose context was used
   *        to install the bundle. Otherwise it is the bundle itself.
   */
  BundleEvent(Type type, const Bundle& bundle, const Bundle& origin);

  BundleEvent(const BundleEvent& other);

  BundleEvent& operator=(const BundleEvent& other);

  /**
   * Returns the bundle which had a lifecycle change.
   *
   * @return The bundle that had a change occur in its lifecycle.
   */
  Bundle GetBundle() const;

  /**
   * Returns the type of lifecyle event. The type values are:
   * <ul>
   * <li>{@link #INSTALLED}
   * <li>{@link #RESOLVED}
   * <li>{@link #LAZY_ACTIVATION}
   * <li>{@link #STARTING}
   * <li>{@link #STARTED}
   * <li>{@link #STOPPING}
   * <li>{@link #STOPPED}
   * <li>{@link #UNRESOLVED}
   * <li>{@link #UNINSTALLED}
   * </ul>
   *
   * @return The type of lifecycle event.
   */
  Type GetType() const;

  /**
   * Returns the bundle that was the origin of the event.
   *
   * <p>
   * For the event type {@link #INSTALLED}, this is the bundle whose context
   * was used to install the bundle. Otherwise it is the bundle itself.
   *
   * @return The bundle that was the origin of the event.
   */
  Bundle GetOrigin() const;

  /**
   * Compares two bundle events for equality.
   *
   * @param evt The bundle event to compare this event with.
   * @return \c true if both events originate from the same bundle, describe
   *         a life-cycle change for the same bundle, and are of the same type.
   *         \c false otherwise. Two invalid bundle events are considered to
   *         be equal.
   */
  bool operator==(const BundleEvent& evt) const;
};

/**
 * \ingroup MicroServices
 * @{
 */
US_Core_EXPORT std::ostream& operator<<(std::ostream& os, BundleEvent::Type eventType);
US_Core_EXPORT std::ostream& operator<<(std::ostream& os, const BundleEvent& event);
/** @}*/

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // USBUNDLEEVENT_H
