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


#ifndef USSERVICETRACKERPRIVATE_H
#define USSERVICETRACKERPRIVATE_H

#include "usServiceReference.h"
#include "usLDAPFilter.h"
#include "usThreads_p.h"

namespace us {

/**
 * \ingroup MicroServices
 */
template<class S, class TTT>
class ServiceTrackerPrivate : MultiThreaded<>
{

public:

  typedef typename TTT::TrackedType T;
  typedef typename TTT::TrackedParmType TrackedParmType;

  ServiceTrackerPrivate(ServiceTracker<S,T>* st,
                        BundleContext* context,
                        const ServiceReference<S>& reference,
                        ServiceTrackerCustomizer<S,T>* customizer);

  ServiceTrackerPrivate(ServiceTracker<S,T>* st,
                        BundleContext* context, const std::string& clazz,
                        ServiceTrackerCustomizer<S,T>* customizer);

  ServiceTrackerPrivate(ServiceTracker<S,T>* st,
                        BundleContext* context, const LDAPFilter& filter,
                        ServiceTrackerCustomizer<S,T>* customizer);

  ~ServiceTrackerPrivate();

  /**
   * Returns the list of initial <code>ServiceReference</code>s that will be
   * tracked by this <code>ServiceTracker</code>.
   *
   * @param className The class name with which the service was registered, or
   *        <code>null</code> for all services.
   * @param filterString The filter criteria or <code>null</code> for all
   *        services.
   * @return The list of initial <code>ServiceReference</code>s.
   * @throws std::invalid_argument If the specified filterString has an
   *         invalid syntax.
   */
  std::vector<ServiceReference<S> > GetInitialReferences(const std::string& className,
                                                         const std::string& filterString);

  void GetServiceReferences_unlocked(std::vector<ServiceReference<S>>& refs, TrackedService<S,TTT>* t) const;

  /* set this to true to compile in debug messages */
  static const bool DEBUG_OUTPUT; // = false;

  /**
   * The Bundle Context used by this <code>ServiceTracker</code>.
   */
  BundleContext* const context;

  /**
   * The filter used by this <code>ServiceTracker</code> which specifies the
   * search criteria for the services to track.
   */
  LDAPFilter filter;

  /**
   * The <code>ServiceTrackerCustomizer</code> for this tracker.
   */
  ServiceTrackerCustomizer<S,T>* customizer;

  /**
   * Filter string for use when adding the ServiceListener. If this field is
   * set, then certain optimizations can be taken since we don't have a user
   * supplied filter.
   */
  std::string listenerFilter;

  /**
   * Class name to be tracked. If this field is set, then we are tracking by
   * class name.
   */
  std::string trackClass;

  /**
   * Reference to be tracked. If this field is set, then we are tracking a
   * single ServiceReference.
   */
  ServiceReference<S> trackReference;

  /**
   * Tracked services: <code>ServiceReference</code> -> customized Object and
   * <code>ServiceListenerEntry</code> object
   */
  std::unique_ptr<TrackedService<S,TTT>> trackedService;

  /**
   * Accessor method for the current TrackedService object. This method is only
   * intended to be used by the unsynchronized methods which do not modify the
   * trackedService field.
   *
   * @return The current Tracked object.
   */
  TrackedService<S,TTT>* Tracked() const;

  /**
   * Called by the TrackedService object whenever the set of tracked services is
   * modified. Clears the cache.
   */
  /*
   * This method must not be synchronized since it is called by TrackedService while
   * TrackedService is synchronized. We don't want synchronization interactions
   * between the listener thread and the user thread.
   */
  void Modified();

  /**
   * Cached ServiceReference for getServiceReference.
   */
  mutable ServiceReference<S> cachedReference;

  /**
   * Cached service object for GetService.
   */
  mutable std::shared_ptr<TrackedParmType> cachedService;


private:

  inline ServiceTracker<S,T>* q_func()
  {
    return static_cast<ServiceTracker<S,T> *>(q_ptr);
  }

  inline const ServiceTracker<S,T>* q_func() const
  {
    return static_cast<const ServiceTracker<S,T> *>(q_ptr);
  }

  friend class ServiceTracker<S,T>;

  ServiceTracker<S,T> * const q_ptr;

};

}

#include "usServiceTrackerPrivate.tpp"

#endif // USSERVICETRACKERPRIVATE_H
