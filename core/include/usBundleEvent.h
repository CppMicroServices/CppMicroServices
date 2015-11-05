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
    UNINSTALLED

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
   * @param bundle The bundle which had a lifecycle change.
   */
  BundleEvent(Type type, const std::shared_ptr<Bundle>& bundle);

  BundleEvent(const BundleEvent& other);

  BundleEvent& operator=(const BundleEvent& other);

  /**
   * Returns the bundle which had a lifecycle change.
   *
   * @return The bundle that had a change occur in its lifecycle.
   */
  std::shared_ptr<Bundle> GetBundle() const;

  /**
   * Returns the type of lifecyle event. The type values are:
   * <ul>
   * <li>{@link #INSTALLED}
   * <li>{@link #STARTING}
   * <li>{@link #STARTED}
   * <li>{@link #STOPPING}
   * <li>{@link #STOPPED}
   * <li>{@link #UNINSTALLED}
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
US_Core_EXPORT std::ostream& operator<<(std::ostream& os, BundleEvent::Type eventType);
US_Core_EXPORT std::ostream& operator<<(std::ostream& os, const BundleEvent& event);
/** @}*/

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // USBUNDLEEVENT_H
