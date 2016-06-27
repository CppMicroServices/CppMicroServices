/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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

#ifndef USFRAMEWORKEVENT_H
#define USFRAMEWORKEVENT_H

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
class FrameworkEventData;

/**
 * \ingroup MicroServices
 *
 * An event from the Micro Services framework describing a Framework event.
 * <p>
 * <code>FrameworkEvent</code> objects are delivered to listeners connected
 * via BundleContext::AddFrameworkListener() when an event occurs within the
 * Framework which a user would be interested in. A <code>Type</code> code 
 * is used to identify the event type for future extendability.
 *
 * @see BundleContext#AddFrameworkListener
 */
class US_Core_EXPORT FrameworkEvent
{

  SharedDataPointer<FrameworkEventData> d;

public:

  /**
   * A type code used to identify the event type for future extendability.
   */
  enum class Type : unsigned int {

    /**
     * The Framework has started.
     *
     * <p>
     * This event is fired when the Framework has started after all installed
     * bundles that are marked to be started have been started. The source of
     * this event is the System Bundle.
     *
     */
    FRAMEWORK_STARTED	= 0x00000001,

    /**
     * The Framework has been started.
     * <p>
     * The Framework's
     * \link BundleActivator::Start(BundleContext*) BundleActivator Start\endlink method
     * has been executed.
     */
    FRAMEWORK_ERROR	= 0x00000002,

    /**
     * A warning has occurred.
     *
     * <p>
     * There was a warning associated with a bundle.
     *
     */
    FRAMEWORK_WARNING	= 0x00000010,

    /**
     * An informational event has occurred.
     *
     * <p>
     * There was an informational event associated with a bundle.
     */
    FRAMEWORK_INFO = 0x00000020,

	/**
	 * A warning has occurred.
	 * <p>
	 * There was a warning associated with a bundle.
	 */
    FRAMEWORK_WARNING = 16,

    /**
     * The Framework has been stopped.
     * <p>
     * The Framework's
     * \link BundleActivator::Stop(BundleContext*) BundleActivator Stop\endlink method
     * has been executed.
     */
    FRAMEWORK_STOPPED	= 0x00000040,

    /**
     * The Framework is about to be stopped.
     * <p>
     * The Framework's
     * \link BundleActivator::Stop(BundleContext*) BundleActivator Stop\endlink method
     * is about to be called.
     */
    FRAMEWORK_STOPPED_UPDATE = 0x00000080,

    /**
     * The Framework did not stop before the wait timeout expired. 
	 * <p>
	 * This event is fired when the Framework did not stop before the wait timeout expired. 
	 * The source of this event is the System Bundle.
     */
    FRAMEWORK_WAIT_TIMEDOUT	= 0x00000200

  };

  /**
   * Creates an invalid instance.
   */
  FrameworkEvent();

  ~FrameworkEvent();

  /**
   * Returns <code>false</code> if the FrameworkEvent is empty (i.e invalid) and
   * <code>true</code> if the FrameworkEvent is not null and contains valid data.
   */
  bool operator!() const;

  /**
   * Creates a Framework event of the specified type.
   *
   * @param type The event type.
   * @param bundle The bundle associated with the event. This bundle is also the source of the event.
   * @param message The message associated with the event.
   * @param exception The exception associated with this event. Should be nullptr if there is no exception.
   */
  FrameworkEvent(Type type, const Bundle& bundle, const std::string& message, const std::exception_ptr exception = nullptr);

  FrameworkEvent(const FrameworkEvent& other);

  FrameworkEvent& operator=(const FrameworkEvent& other);

  /**
   * Returns the bundle associated with the event.
   *
   * @return The bundle associated with the event.
   */
  std::shared_ptr<Bundle> GetBundle() const;

  /**
   * Returns the message associated with the event.
   *
   *@return the message associated with the event.
   */
  std::string GetMessage() const;

  /**
   * Returns the exception associated with this event.
   *
   * @remarks Use <code>std::rethrow_exception</code> to throw the exception returned.
   *
   * @return The exception. May be <code>nullptr</code> if there is no related exception.
   */
  std::exception_ptr GetThrowable() const;

  /**
   * Returns the type of lifecyle event. The type values are:
   * <ul>
   * <li>{@link #FRAMEWORK_STARTED}
   * <li>{@link #FRAMEWORK_ERROR}
   * <li>{@link #FRAMEWORK_WARNING}
   * <li>{@link #FRAMEWORK_INFO}
   * <li>{@link #FRAMEWORK_STOPPED}
   * <li>{@link #FRAMEWORK_STOPPED_UPDATE}
   * <li>{@link #FRAMEWORK_WAIT_TIMEDOUT}
   * </ul>
   *
   * @return The type of Framework event.
   */
  Type GetType() const;

};

/**
 * \ingroup MicroServices
 * @{
 */
US_Core_EXPORT std::ostream& operator<<(std::ostream& os, FrameworkEvent::Type eventType);
US_Core_EXPORT std::ostream& operator<<(std::ostream& os, const std::exception_ptr ex);
US_Core_EXPORT std::ostream& operator<<(std::ostream& os, const FrameworkEvent& evt);
/** @}*/

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // USFRAMEWORKEVENT_H
