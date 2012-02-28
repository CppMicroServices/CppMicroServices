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


#ifndef USSERVICETRACKERPRIVATE_H
#define USSERVICETRACKERPRIVATE_H

#include "usServiceReference.h"
#include "usLDAPFilter.h"


US_BEGIN_NAMESPACE

/**
 * \ingroup MicroServices
 */
template<class S, class T>
class ServiceTrackerPrivate : US_DEFAULT_THREADING<ServiceTrackerPrivate<S,T> >
{

public:

  ServiceTrackerPrivate(ServiceTracker<S,T>* st,
                        ModuleContext* context,
                        const ServiceReference& reference,
                        ServiceTrackerCustomizer<T>* customizer);

  ServiceTrackerPrivate(ServiceTracker<S,T>* st,
                        ModuleContext* context, const std::string& clazz,
                        ServiceTrackerCustomizer<T>* customizer);

  ServiceTrackerPrivate(ServiceTracker<S,T>* st,
                        ModuleContext* context, const LDAPFilter& filter,
                        ServiceTrackerCustomizer<T>* customizer);

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
  std::list<ServiceReference> GetInitialReferences(const std::string& className,
                                                   const std::string& filterString);

  void GetServiceReferences_unlocked(std::list<ServiceReference>& refs, TrackedService<S,T>* t) const;

  /* set this to true to compile in debug messages */
  static const bool DEBUG; // = false;

  /**
   * The Module Context used by this <code>ServiceTracker</code>.
   */
  ModuleContext* const context;

  /**
   * The filter used by this <code>ServiceTracker</code> which specifies the
   * search criteria for the services to track.
   */
  LDAPFilter filter;

  /**
   * The <code>ServiceTrackerCustomizer</code> for this tracker.
   */
  ServiceTrackerCustomizer<T>* customizer;

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
  ServiceReference trackReference;

  /**
   * Tracked services: <code>ServiceReference</code> -> customized Object and
   * <code>ServiceListenerEntry</code> object
   */
  TrackedService<S,T>* trackedService;

  /**
   * Accessor method for the current TrackedService object. This method is only
   * intended to be used by the unsynchronized methods which do not modify the
   * trackedService field.
   *
   * @return The current Tracked object.
   */
  TrackedService<S,T>* Tracked() const;

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
  mutable ServiceReference cachedReference;

  /**
   * Cached service object for GetService.
   *
   * This field is volatile since it is accessed by multiple threads.
   */
  mutable T volatile cachedService;


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

US_END_NAMESPACE

#include "usServiceTrackerPrivate.tpp"

#endif // USSERVICETRACKERPRIVATE_H
