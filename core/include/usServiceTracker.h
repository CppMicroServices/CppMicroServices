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


#ifndef USSERVICETRACKER_H
#define USSERVICETRACKER_H

#include <map>

#include "usServiceReference.h"
#include "usServiceTrackerCustomizer.h"
#include "usLDAPFilter.h"

US_BEGIN_NAMESPACE

template<class S, class T> class TrackedService;
template<class S, class T> class ServiceTrackerPrivate;
class ModuleContext;

/**
 * \ingroup MicroServices
 *
 * A base class template for type traits for objects tracked by a
 * ServiceTracker instance. It provides the \c TrackedType typedef
 * and two dummy method definitions.
 *
 * Tracked type traits (TTT) classes must additionally provide the
 * following methods:
 *
 * <ul>
 *   <li><em>static bool IsValid(const TrackedType& t)</em> Returns \c true if \c t is a valid object, \c false otherwise.</li>
 *   <li><em>static void Dispose(TrackedType& t)</em> Clears any resources held by the tracked object \c t.</li>
 *   <li><em>static TrackedType DefaultValue()</em> Returns the default value for newly created tracked objects.</li>
 * </ul>
 *
 * @tparam T The type of the tracked object.
 * @tparam TTT The tracked type traits class deriving from this class.
 *
 * @see ServiceTracker
 */
template<class T, class TTT>
struct TrackedTypeTraitsBase
{
  typedef T TrackedType;

  // Needed for S == void
  static TrackedType ConvertToTrackedType(const InterfaceMap&)
  {
    throw std::runtime_error("A custom ServiceTrackerCustomizer instance is required for custom tracked objects.");
    //return TTT::DefaultValue();
  }

  // Needed for S != void
  static TrackedType ConvertToTrackedType(void*)
  {
    throw std::runtime_error("A custom ServiceTrackerCustomizer instance is required for custom tracked objects.");
    //return TTT::DefaultValue();
  }
};

/// \cond
template<class S, class T>
struct TrackedTypeTraits;
/// \endcond

/**
 * \ingroup MicroServices
 *
 * Default type traits for custom tracked objects of pointer type.
 *
 * Use this tracked type traits template for custom tracked objects of
 * pointer type with the ServiceTracker class.
 *
 * @tparam S The type of the service being tracked.
 * @tparam T The type of the tracked object.
 */
template<class S, class T>
struct TrackedTypeTraits<S,T*> : public TrackedTypeTraitsBase<T*,TrackedTypeTraits<S,T*> >
{
  typedef T* TrackedType;

  static bool IsValid(const TrackedType& t)
  {
    return t != NULL;
  }

  static TrackedType DefaultValue()
  {
    return NULL;
  }

  static void Dispose(TrackedType& t)
  {
    t = 0;
  }
};

/// \cond
template<class S>
struct TrackedTypeTraits<S,S*>
{
  typedef S* TrackedType;

  static bool IsValid(const TrackedType& t)
  {
    return t != NULL;
  }

  static TrackedType DefaultValue()
  {
    return NULL;
  }

  static void Dispose(TrackedType& t)
  {
    t = 0;
  }

  static TrackedType ConvertToTrackedType(S* s)
  {
    return s;
  }
};
/// \endcond

/// \cond
/*
 * This specialization is "special" because the tracked type is not
 * void* (as specified in the second template parameter) but InterfaceMap.
 * This is in line with the ModuleContext::GetService(...) overloads to
 * return either S* or InterfaceMap dependening on the template parameter.
 */
template<>
struct TrackedTypeTraits<void,void*>
{
  typedef InterfaceMap TrackedType;

  static bool IsValid(const TrackedType& t)
  {
    return !t.empty();
  }

  static TrackedType DefaultValue()
  {
    return TrackedType();
  }

  static void Dispose(TrackedType& t)
  {
    t.clear();
  }

  static TrackedType ConvertToTrackedType(const InterfaceMap& im)
  {
    return im;
  }
};
/// \endcond

/**
 * \ingroup MicroServices
 *
 * The <code>ServiceTracker</code> class simplifies using services from the
 * framework's service registry.
 * <p>
 * A <code>ServiceTracker</code> object is constructed with search criteria and
 * a <code>ServiceTrackerCustomizer</code> object. A <code>ServiceTracker</code>
 * can use a <code>ServiceTrackerCustomizer</code> to customize the service
 * objects to be tracked. The <code>ServiceTracker</code> can then be opened to
 * begin tracking all services in the framework's service registry that match
 * the specified search criteria. The <code>ServiceTracker</code> correctly
 * handles all of the details of listening to <code>ServiceEvent</code>s and
 * getting and ungetting services.
 * <p>
 * The <code>GetServiceReferences</code> method can be called to get references
 * to the services being tracked. The <code>GetService</code> and
 * <code>GetServices</code> methods can be called to get the service objects for
 * the tracked service.
 *
 * \note The <code>ServiceTracker</code> class is thread-safe. It does not call a
 *       <code>ServiceTrackerCustomizer</code> while holding any locks.
 *       <code>ServiceTrackerCustomizer</code> implementations must also be
 *       thread-safe.
 *
 * Customization of the services to be tracked requires a custom tracked type traits
 * class if the custom tracked type is not a pointer type. To customize a tracked
 * service using a custom type with value-semantics like
 * \snippet uServices-servicetracker/main.cpp tt
 * the custom tracked type traits class should look like this:
 * \snippet uServices-servicetracker/main.cpp ttt
 *
 * For a custom tracked type, a ServiceTrackerCustomizer is required, which knows
 * how to associate the tracked service with the custom tracked type:
 * \snippet uServices-servicetracker/main.cpp customizer
 * The custom tracking type traits class and customizer can now be used to instantiate
 * a ServiceTracker:
 * \snippet uServices-servicetracker/main.cpp tracker
 *
 * If the custom tracked type is a pointer type, a suitable tracked type traits
 * template is provided by the framework and only a ServiceTrackerCustomizer needs
 * to be provided:
 * \snippet uServices-servicetracker/main.cpp tracker2
 *
 *
 * @tparam S The type of the service being tracked. The type S* must be an
 *         assignable datatype.
 * @tparam TTT Type traits of the tracked object. The type traits class provides
 *         information about the customized service object, see TrackedTypeTraitsBase.
 *
 * @remarks This class is thread safe.
 */
template<class S, class TTT = TrackedTypeTraits<S,S*> >
class ServiceTracker : protected ServiceTrackerCustomizer<S,typename TTT::TrackedType>
{
public:

  /// The type of the service being tracked
  typedef S ServiceType;
  /// The type of the tracked object
  typedef typename TTT::TrackedType T;

  typedef ServiceReference<S> ServiceReferenceType;

  typedef std::map<ServiceReference<S>,T> TrackingMap;

  ~ServiceTracker();

  /**
   * Create a <code>ServiceTracker</code> on the specified
   * <code>ServiceReference</code>.
   *
   * <p>
   * The service referenced by the specified <code>ServiceReference</code>
   * will be tracked by this <code>ServiceTracker</code>.
   *
   * @param context The <code>ModuleContext</code> against which the tracking
   *        is done.
   * @param reference The <code>ServiceReference</code> for the service to be
   *        tracked.
   * @param customizer The customizer object to call when services are added,
   *        modified, or removed in this <code>ServiceTracker</code>. If
   *        customizer is <code>null</code>, then this
   *        <code>ServiceTracker</code> will be used as the
   *        <code>ServiceTrackerCustomizer</code> and this
   *        <code>ServiceTracker</code> will call the
   *        <code>ServiceTrackerCustomizer</code> methods on itself.
   */
  ServiceTracker(ModuleContext* context,
                 const ServiceReferenceType& reference,
                 ServiceTrackerCustomizer<S,T>* customizer = 0);

  /**
   * Create a <code>ServiceTracker</code> on the specified class name.
   *
   * <p>
   * Services registered under the specified class name will be tracked by
   * this <code>ServiceTracker</code>.
   *
   * @param context The <code>ModuleContext</code> against which the tracking
   *        is done.
   * @param clazz The class name of the services to be tracked.
   * @param customizer The customizer object to call when services are added,
   *        modified, or removed in this <code>ServiceTracker</code>. If
   *        customizer is <code>null</code>, then this
   *        <code>ServiceTracker</code> will be used as the
   *        <code>ServiceTrackerCustomizer</code> and this
   *        <code>ServiceTracker</code> will call the
   *        <code>ServiceTrackerCustomizer</code> methods on itself.
   */
  ServiceTracker(ModuleContext* context, const std::string& clazz,
                 ServiceTrackerCustomizer<S,T>* customizer = 0);

  /**
   * Create a <code>ServiceTracker</code> on the specified
   * <code>LDAPFilter</code> object.
   *
   * <p>
   * Services which match the specified <code>LDAPFilter</code> object will be
   * tracked by this <code>ServiceTracker</code>.
   *
   * @param context The <code>ModuleContext</code> against which the tracking
   *        is done.
   * @param filter The <code>LDAPFilter</code> to select the services to be
   *        tracked.
   * @param customizer The customizer object to call when services are added,
   *        modified, or removed in this <code>ServiceTracker</code>. If
   *        customizer is null, then this <code>ServiceTracker</code> will be
   *        used as the <code>ServiceTrackerCustomizer</code> and this
   *        <code>ServiceTracker</code> will call the
   *        <code>ServiceTrackerCustomizer</code> methods on itself.
   */
  ServiceTracker(ModuleContext* context, const LDAPFilter& filter,
                 ServiceTrackerCustomizer<S,T>* customizer = 0);

  /**
   * Create a <code>ServiceTracker</code> on the class template
   * argument S.
   *
   * <p>
   * Services registered under the interface name of the class template
   * argument S will be tracked by this <code>ServiceTracker</code>.
   *
   * @param context The <code>ModuleContext</code> against which the tracking
   *        is done.
   * @param customizer The customizer object to call when services are added,
   *        modified, or removed in this <code>ServiceTracker</code>. If
   *        customizer is null, then this <code>ServiceTracker</code> will be
   *        used as the <code>ServiceTrackerCustomizer</code> and this
   *        <code>ServiceTracker</code> will call the
   *        <code>ServiceTrackerCustomizer</code> methods on itself.
   */
  ServiceTracker(ModuleContext* context, ServiceTrackerCustomizer<S,T>* customizer = 0);

  /**
   * Open this <code>ServiceTracker</code> and begin tracking services.
   *
   * <p>
   * Services which match the search criteria specified when this
   * <code>ServiceTracker</code> was created are now tracked by this
   * <code>ServiceTracker</code>.
   *
   * @throws std::logic_error If the <code>ModuleContext</code>
   *         with which this <code>ServiceTracker</code> was created is no
   *         longer valid.
   */
  virtual void Open();

  /**
   * Close this <code>ServiceTracker</code>.
   *
   * <p>
   * This method should be called when this <code>ServiceTracker</code> should
   * end the tracking of services.
   *
   * <p>
   * This implementation calls GetServiceReferences() to get the list
   * of tracked services to remove.
   */
  virtual void Close();

  /**
   * Wait for at least one service to be tracked by this
   * <code>ServiceTracker</code>. This method will also return when this
   * <code>ServiceTracker</code> is closed.
   *
   * <p>
   * It is strongly recommended that <code>WaitForService</code> is not used
   * during the calling of the <code>ModuleActivator</code> methods.
   * <code>ModuleActivator</code> methods are expected to complete in a short
   * period of time.
   *
   * <p>
   * This implementation calls GetService() to determine if a service
   * is being tracked.
   *
   * @return Returns the result of GetService().
   */
  virtual T WaitForService(unsigned long timeoutMillis = 0);

  /**
   * Return a list of <code>ServiceReference</code>s for all services being
   * tracked by this <code>ServiceTracker</code>.
   *
   * @return List of <code>ServiceReference</code>s.
   */
  virtual std::vector<ServiceReferenceType> GetServiceReferences() const;

  /**
   * Returns a <code>ServiceReference</code> for one of the services being
   * tracked by this <code>ServiceTracker</code>.
   *
   * <p>
   * If multiple services are being tracked, the service with the highest
   * ranking (as specified in its <code>service.ranking</code> property) is
   * returned. If there is a tie in ranking, the service with the lowest
   * service ID (as specified in its <code>service.id</code> property); that
   * is, the service that was registered first is returned. This is the same
   * algorithm used by <code>ModuleContext::GetServiceReference()</code>.
   *
   * <p>
   * This implementation calls GetServiceReferences() to get the list
   * of references for the tracked services.
   *
   * @return A <code>ServiceReference</code> for a tracked service.
   * @throws ServiceException if no services are being tracked.
   */
  virtual ServiceReferenceType GetServiceReference() const;

  /**
   * Returns the service object for the specified
   * <code>ServiceReference</code> if the specified referenced service is
   * being tracked by this <code>ServiceTracker</code>.
   *
   * @param reference The reference to the desired service.
   * @return A service object or <code>null</code> if the service referenced
   *         by the specified <code>ServiceReference</code> is not being
   *         tracked.
   */
  virtual T GetService(const ServiceReferenceType& reference) const;

  /**
   * Return a list of service objects for all services being tracked by this
   * <code>ServiceTracker</code>.
   *
   * <p>
   * This implementation calls GetServiceReferences() to get the list
   * of references for the tracked services and then calls
   * GetService(const ServiceReference&) for each reference to get the
   * tracked service object.
   *
   * @return A list of service objects or an empty list if no services
   *         are being tracked.
   */
  virtual std::vector<T> GetServices() const;

  /**
   * Returns a service object for one of the services being tracked by this
   * <code>ServiceTracker</code>.
   *
   * <p>
   * If any services are being tracked, this implementation returns the result
   * of calling <code>%GetService(%GetServiceReference())</code>.
   *
   * @return A service object or <code>null</code> if no services are being
   *         tracked.
   */
  virtual T GetService() const;

  /**
   * Remove a service from this <code>ServiceTracker</code>.
   *
   * The specified service will be removed from this
   * <code>ServiceTracker</code>. If the specified service was being tracked
   * then the <code>ServiceTrackerCustomizer::RemovedService</code> method will
   * be called for that service.
   *
   * @param reference The reference to the service to be removed.
   */
  virtual void Remove(const ServiceReferenceType& reference);

  /**
   * Return the number of services being tracked by this
   * <code>ServiceTracker</code>.
   *
   * @return The number of services being tracked.
   */
  virtual int Size() const;

  /**
   * Returns the tracking count for this <code>ServiceTracker</code>.
   *
   * The tracking count is initialized to 0 when this
   * <code>ServiceTracker</code> is opened. Every time a service is added,
   * modified or removed from this <code>ServiceTracker</code>, the tracking
   * count is incremented.
   *
   * <p>
   * The tracking count can be used to determine if this
   * <code>ServiceTracker</code> has added, modified or removed a service by
   * comparing a tracking count value previously collected with the current
   * tracking count value. If the value has not changed, then no service has
   * been added, modified or removed from this <code>ServiceTracker</code>
   * since the previous tracking count was collected.
   *
   * @return The tracking count for this <code>ServiceTracker</code> or -1 if
   *         this <code>ServiceTracker</code> is not open.
   */
  virtual int GetTrackingCount() const;

  /**
   * Return a sorted map of the <code>ServiceReference</code>s and
   * service objects for all services being tracked by this
   * <code>ServiceTracker</code>. The map is sorted in natural order
   * of <code>ServiceReference</code>. That is, the last entry is the service
   * with the highest ranking and the lowest service id.
   *
   * @param tracked A <code>TrackingMap</code> with the <code>ServiceReference</code>s
   *         and service objects for all services being tracked by this
   *         <code>ServiceTracker</code>. If no services are being tracked,
   *         then the returned map is empty.
   */
  virtual void GetTracked(TrackingMap& tracked) const;

  /**
   * Return if this <code>ServiceTracker</code> is empty.
   *
   * @return <code>true</code> if this <code>ServiceTracker</code> is not tracking any
   *         services.
   */
  virtual bool IsEmpty() const;

protected:

  /**
   * Default implementation of the
   * <code>ServiceTrackerCustomizer::AddingService</code> method.
   *
   * <p>
   * This method is only called when this <code>ServiceTracker</code> has been
   * constructed with a <code>null</code> ServiceTrackerCustomizer argument.
   *
   * <p>
   * This implementation returns the result of calling <code>GetService</code>
   * on the <code>ModuleContext</code> with which this
   * <code>ServiceTracker</code> was created passing the specified
   * <code>ServiceReference</code>.
   * <p>
   * This method can be overridden in a subclass to customize the service
   * object to be tracked for the service being added. In that case, take care
   * not to rely on the default implementation of
   * \link RemovedService(const ServiceReferenceType&, T service) removedService\endlink
   * to unget the service.
   *
   * @param reference The reference to the service being added to this
   *        <code>ServiceTracker</code>.
   * @return The service object to be tracked for the service added to this
   *         <code>ServiceTracker</code>.
   * @see ServiceTrackerCustomizer::AddingService(const ServiceReference&)
   */
  T AddingService(const ServiceReferenceType& reference);

  /**
   * Default implementation of the
   * <code>ServiceTrackerCustomizer::ModifiedService</code> method.
   *
   * <p>
   * This method is only called when this <code>ServiceTracker</code> has been
   * constructed with a <code>null</code> ServiceTrackerCustomizer argument.
   *
   * <p>
   * This implementation does nothing.
   *
   * @param reference The reference to modified service.
   * @param service The service object for the modified service.
   * @see ServiceTrackerCustomizer::ModifiedService(const ServiceReference&, T)
   */
  void ModifiedService(const ServiceReferenceType& reference, T service);

  /**
   * Default implementation of the
   * <code>ServiceTrackerCustomizer::RemovedService</code> method.
   *
   * <p>
   * This method is only called when this <code>ServiceTracker</code> has been
   * constructed with a <code>null</code> ServiceTrackerCustomizer argument.
   *
   * <p>
   * This implementation calls <code>UngetService</code>, on the
   * <code>ModuleContext</code> with which this <code>ServiceTracker</code>
   * was created, passing the specified <code>ServiceReference</code>.
   * <p>
   * This method can be overridden in a subclass. If the default
   * implementation of \link AddingService(const ServiceReferenceType&) AddingService\endlink
   * method was used, this method must unget the service.
   *
   * @param reference The reference to removed service.
   * @param service The service object for the removed service.
   * @see ServiceTrackerCustomizer::RemovedService(const ServiceReferenceType&, T)
   */
  void RemovedService(const ServiceReferenceType& reference, T service);

private:

  typedef ServiceTracker<S,TTT> _ServiceTracker;
  typedef TrackedService<S,TTT> _TrackedService;
  typedef ServiceTrackerPrivate<S,TTT> _ServiceTrackerPrivate;
  typedef ServiceTrackerCustomizer<S,T> _ServiceTrackerCustomizer;

  friend class TrackedService<S,TTT>;
  friend class ServiceTrackerPrivate<S,TTT>;

  _ServiceTrackerPrivate* const d;
};

US_END_NAMESPACE

#include "usServiceTracker.tpp"

#endif // USSERVICETRACKER_H
