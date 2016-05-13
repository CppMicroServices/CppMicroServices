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

#ifndef USFRAMEWORKEVENT_H
#define USFRAMEWORKEVENT_H

#include "usCoreExport.h"
#include "usSharedData.h"

#include <memory>

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
 * A general event from the Framework.
 *
 * <p>
 * FrameworkEvent objects are delivered to {@code FrameworkListener}s
 * when a general event occurs within the CppMicroServices environment.
 *
 * @see FrameworkListener
 */
class US_Core_EXPORT FrameworkEvent
{

  SharedDataPointer<FrameworkEventData> d;

public:

  enum Type {

    /**
     * The Framework has started.
     *
     * <p>
     * This event is fired when the Framework has started after all installed
     * bundles that are marked to be started have been started. The source of
     * this event is the System Bundle.
     *
     */
    STARTED	= 0x00000001,

    /**
     * An error has occurred.
     *
     * <p>
     * There was an error associated with a bundle.
     */
    ERROR	= 0x00000002,

    /**
     * A warning has occurred.
     *
     * <p>
     * There was a warning associated with a bundle.
     *
     */
    WARNING	= 0x00000010,

    /**
     * An informational event has occurred.
     *
     * <p>
     * There was an informational event associated with a bundle.
     */
    INFO = 0x00000020,

    /**
     * The Framework has stopped.
     *
     * <p>
     * This event is fired when the Framework has been stopped because of a stop
     * operation on the system bundle. The source of this event is the System
     * Bundle.
     */
    STOPPED	= 0x00000040,

    /**
     * The Framework has stopped during update.
     *
     * <p>
     * This event is fired when the Framework has been stopped because of an
     * update operation on the system bundle. The Framework will be restarted
     * after this event is fired. The source of this event is the System Bundle.
     */
    STOPPED_UPDATE = 0x00000080,

    /**
     * The Framework did not stop before the wait timeout expired.
     *
     * <p>
     * This event is fired when the Framework did not stop before the wait
     * timeout expired. The source of this event is the System Bundle.
     */
    WAIT_TIMEDOUT	= 0x00000200

  };

  /**
   * Creates an invalid instance.
   */
  FrameworkEvent();

  ~FrameworkEvent();

  /**
   * Can be used to check if this FrameworkEvent instance is valid,
   * or if it has been constructed using the default constructor.
   *
   * @return <code>true</code> if this event object is valid,
   *         <code>false</code> otherwise.
   */
  bool IsNull() const;

  FrameworkEvent(const FrameworkEvent& other);

  FrameworkEvent& operator=(const FrameworkEvent& other);

  /**
   * Creates a Framework event regarding the specified bundle.
   *
   * @param type The event type.
   * @param bundle The event source.
   * @param throwable The related exception. This argument may be {@code null}
   *        if there is no related exception.
   */
  FrameworkEvent(Type type, const Bundle& bundle, const std::exception_ptr& e);

  /**
   * Returns the exception related to this event.
   *
   * @return The related exception or {@code null} if none.
   */
  std::exception_ptr GetException() const;

  /**
   * Returns the bundle associated with the event. This bundle is also the
   * source of the event.
   *
   * @return The bundle associated with the event.
   */
  Bundle GetBundle() const;

  /**
   * Returns the type of framework event.
   * <p>
   * The type values are:
   * <ul>
   * <li>{@link #STARTED}
   * <li>{@link #ERROR}
   * <li>{@link #WARNING}
   * <li>{@link #INFO}
   * <li>{@link #STOPPED}
   * <li>{@link #STOPPED_UPDATE}
   * <li>{@link #WAIT_TIMEDOUT}
   * </ul>
   *
   * @return The type of state change.
   */
  Type GetType() const;

};

/**
 * \ingroup MicroServices
 * @{
 */
US_Core_EXPORT std::ostream& operator<<(std::ostream& os, FrameworkEvent::Type eventType);
US_Core_EXPORT std::ostream& operator<<(std::ostream& os, const FrameworkEvent& event);
/** @}*/

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // USFRAMEWORKEVENT_H
