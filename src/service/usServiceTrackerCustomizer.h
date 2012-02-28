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


#ifndef USSERVICETRACKERCUSTOMIZER_H
#define USSERVICETRACKERCUSTOMIZER_H

#include "usServiceReference.h"

US_BEGIN_NAMESPACE

/**
 * \ingroup MicroServices
 *
 * The <code>ServiceTrackerCustomizer</code> interface allows a
 * <code>ServiceTracker</code> to customize the service objects that are
 * tracked. A <code>ServiceTrackerCustomizer</code> is called when a service is
 * being added to a <code>ServiceTracker</code>. The
 * <code>ServiceTrackerCustomizer</code> can then return an object for the
 * tracked service. A <code>ServiceTrackerCustomizer</code> is also called when
 * a tracked service is modified or has been removed from a
 * <code>ServiceTracker</code>.
 *
 * <p>
 * The methods in this interface may be called as the result of a
 * <code>ServiceEvent</code> being received by a <code>ServiceTracker</code>.
 * Since <code>ServiceEvent</code>s are synchronously delivered,
 * it is highly recommended that implementations of these methods do
 * not register (<code>ModuleContext::RegisterService</code>), modify (
 * <code>ServiceRegistration::SetProperties</code>) or unregister (
 * <code>ServiceRegistration::Unregister</code>) a service while being
 * synchronized on any object.
 *
 * <p>
 * The <code>ServiceTracker</code> class is thread-safe. It does not call a
 * <code>ServiceTrackerCustomizer</code> while holding any locks.
 * <code>ServiceTrackerCustomizer</code> implementations must also be
 * thread-safe.
 *
 * \tparam T The type of the tracked object.
 * \remarks This class is thread safe.
 */
template<class T = US_BASECLASS_NAME*>
struct ServiceTrackerCustomizer {

  virtual ~ServiceTrackerCustomizer() {}

  /**
   * A service is being added to the <code>ServiceTracker</code>.
   *
   * <p>
   * This method is called before a service which matched the search
   * parameters of the <code>ServiceTracker</code> is added to the
   * <code>ServiceTracker</code>. This method should return the service object
   * to be tracked for the specified <code>ServiceReference</code>. The
   * returned service object is stored in the <code>ServiceTracker</code> and
   * is available from the <code>GetService</code> and
   * <code>GetServices</code> methods.
   *
   * @param reference The reference to the service being added to the
   *        <code>ServiceTracker</code>.
   * @return The service object to be tracked for the specified referenced
   *         service or <code>0</code> if the specified referenced service
   *         should not be tracked.
   */
  virtual T AddingService(const ServiceReference& reference) = 0;

  /**
   * A service tracked by the <code>ServiceTracker</code> has been modified.
   *
   * <p>
   * This method is called when a service being tracked by the
   * <code>ServiceTracker</code> has had it properties modified.
   *
   * @param reference The reference to the service that has been modified.
   * @param service The service object for the specified referenced service.
   */
  virtual void ModifiedService(const ServiceReference& reference, T service) = 0;

  /**
   * A service tracked by the <code>ServiceTracker</code> has been removed.
   *
   * <p>
   * This method is called after a service is no longer being tracked by the
   * <code>ServiceTracker</code>.
   *
   * @param reference The reference to the service that has been removed.
   * @param service The service object for the specified referenced service.
   */
  virtual void RemovedService(const ServiceReference& reference, T service) = 0;
};

US_END_NAMESPACE

#endif // USSERVICETRACKERCUSTOMIZER_H
