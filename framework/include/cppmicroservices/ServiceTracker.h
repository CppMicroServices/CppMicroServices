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

#ifndef CPPMICROSERVICES_SERVICETRACKER_H
#define CPPMICROSERVICES_SERVICETRACKER_H

#include <chrono>
#include <map>

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/LDAPFilter.h"
#include "cppmicroservices/ServiceEvent.h"
#include "cppmicroservices/ServiceException.h"
#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/ServiceTrackerCustomizer.h"
#include "cppmicroservices/detail/Log.h"

#include <limits>
#include <stdexcept>
#include <string>

namespace cppmicroservices
{

    namespace detail
    {
        template <class S, class T>
        class TrackedService;
        template <class S, class T>
        class ServiceTrackerPrivate;
    } // namespace detail

    class BundleContext;

    /**
    \defgroup gr_servicetracker ServiceTracker

    \brief Groups ServiceTracker related symbols.
    */

    /**
     * \ingroup MicroServices
     * \ingroup gr_servicetracker
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
     * Customization of the services to be tracked requires the tracked type to be
     * default constructible and convertible to \c bool. To customize a tracked
     * service using a custom type with value-semantics like
     * \snippet uServices-servicetracker/main.cpp tt
     * a custom ServiceTrackerCustomizer is required. It provides code to
     * associate the tracked service with the custom tracked type:
     * \snippet uServices-servicetracker/main.cpp customizer
     * Instantiation of a ServiceTracker with the custom customizer looks like this:
     * \snippet uServices-servicetracker/main.cpp tracker
     *
     * @tparam S The type of the service being tracked. The type S* must be an
     *         assignable datatype.
     * @tparam T The tracked object.
     *
     * @remarks This class is thread safe.
     */
    template <class S, class T = S>
    class ServiceTracker : protected ServiceTrackerCustomizer<S, T>
    {
      public:
        /// The type of the tracked object
        using TrackedParamType = typename ServiceTrackerCustomizer<S, T>::TrackedParamType;

        using TrackingMap = std::unordered_map<ServiceReference<S>, std::shared_ptr<TrackedParamType>>;

        /**
         * Automatically closes the <code>ServiceTracker</code>
         */
        ~ServiceTracker() override
        {
            try
            {
                Close();
            }
            catch (...)
            {
            }
        }

#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4355)
#endif

        /**
         * Create a <code>ServiceTracker</code> on the specified
         * <code>ServiceReference</code>.
         *
         * <p>
         * The service referenced by the specified <code>ServiceReference</code>
         * will be tracked by this <code>ServiceTracker</code>.
         *
         * @param context The <code>BundleContext</code> against which the tracking
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
        ServiceTracker(BundleContext const& context,
                       ServiceReference<S> const& reference,
                       ServiceTrackerCustomizer<S, T>* customizer = nullptr)
            : d(new _ServiceTrackerPrivate(this, context, reference, customizer))
        {
        }

        /**
         * Create a <code>ServiceTracker</code> on the specified class name.
         *
         * <p>
         * Services registered under the specified class name will be tracked by
         * this <code>ServiceTracker</code>.
         *
         * @param context The <code>BundleContext</code> against which the tracking
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
        ServiceTracker(BundleContext const& context,
                       std::string const& clazz,
                       ServiceTrackerCustomizer<S, T>* customizer = nullptr)
            : d(new _ServiceTrackerPrivate(this, context, clazz, customizer))
        {
        }

        /**
         * Create a <code>ServiceTracker</code> on the specified
         * <code>LDAPFilter</code> object.
         *
         * <p>
         * Services which match the specified <code>LDAPFilter</code> object will be
         * tracked by this <code>ServiceTracker</code>.
         *
         * @param context The <code>BundleContext</code> against which the tracking
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
        ServiceTracker(BundleContext const& context,
                       LDAPFilter const& filter,
                       ServiceTrackerCustomizer<S, T>* customizer = nullptr)
            : d(new _ServiceTrackerPrivate(this, context, filter, customizer))
        {
        }

        /**
         * Create a <code>ServiceTracker</code> on the class template
         * argument S.
         *
         * <p>
         * Services registered under the interface name of the class template
         * argument S will be tracked by this <code>ServiceTracker</code>.
         *
         * @param context The <code>BundleContext</code> against which the tracking
         *        is done.
         * @param customizer The customizer object to call when services are added,
         *        modified, or removed in this <code>ServiceTracker</code>. If
         *        customizer is null, then this <code>ServiceTracker</code> will be
         *        used as the <code>ServiceTrackerCustomizer</code> and this
         *        <code>ServiceTracker</code> will call the
         *        <code>ServiceTrackerCustomizer</code> methods on itself.
         * @throws ServiceException If the service interface name is empty.
         */
        ServiceTracker(BundleContext const& context, ServiceTrackerCustomizer<S, T>* customizer = nullptr)
            : d(new _ServiceTrackerPrivate(this, context, us_service_interface_iid<S>(), customizer))
        {
            std::string clazz = us_service_interface_iid<S>();
            if (clazz.empty())
            {
                throw ServiceException("The service interface class has no "
                                       "CPPMICROSERVICES_DECLARE_SERVICE_INTERFACE macro");
            }
        }

#ifdef _MSC_VER
#    pragma warning(pop)
#endif

        /**
         * Open this <code>ServiceTracker</code> and begin tracking services.
         *
         * <p>
         * Services which match the search criteria specified when this
         * <code>ServiceTracker</code> was created are now tracked by this
         * <code>ServiceTracker</code>.
         *
         * @throws std::runtime_error If the <code>BundleContext</code>
         *         with which this <code>ServiceTracker</code> was created is no
         *         longer valid.
         * @throws std::runtime_error If the LDAP filter used to construct
         *         the <code>ServiceTracker</code> contains an invalid filter
         *         expression that cannot be parsed.
         */
        virtual void
        Open()
        {
            std::shared_ptr<_TrackedService> t;
            {
                auto l = d->Lock();
                US_UNUSED(l);
                if (d->trackedService.Load() && !d->Tracked()->closed)
                {
                    return;
                }

                DIAG_LOG(*d->context.GetLogSink()) << "ServiceTracker<S,TTT>::Open: " << d->filter;

                t.reset(new _TrackedService(this, d->customizer));
                try
                {
                    /* Remove if already exists. No-op if it's an invalid (default) token */
                    d->context.RemoveListener(std::move(d->listenerToken));
                    d->listenerToken = d->context.AddServiceListener(
                        std::bind(&_TrackedService::ServiceChanged, t, std::placeholders::_1),
                        d->listenerFilter);
                    std::vector<ServiceReference<S>> references;
                    if (!d->trackClass.empty())
                    {
                        references = d->GetInitialReferences(d->trackClass, std::string());
                    }
                    else
                    {
                        if (d->trackReference.GetBundle())
                        {
                            references.push_back(d->trackReference);
                        }
                        else
                        { /* user supplied filter */
                            references = d->GetInitialReferences(std::string(),
                                                                 (d->listenerFilter.empty()) ? d->filter.ToString()
                                                                                             : d->listenerFilter);
                        }
                    }
                    /* set tracked with the initial references */
                    t->SetInitial(references);
                }
                catch (std::invalid_argument const& e)
                {
                    d->context.RemoveListener(std::move(d->listenerToken));
                    throw std::runtime_error(std::string("unexpected std::invalid_argument exception: ") + e.what());
                }
                d->trackedService.Store(t);
            }
            /* Call tracked outside of synchronized region */
            t->TrackInitial(); /* process the initial references */
        }

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
         *
         * @throws std::runtime_error If the <code>BundleContext</code>
         *         with which this <code>ServiceTracker</code> was created is no
         *         longer valid.
         */
        virtual void
        Close()
        {

            /*
            The call to RemoveListener() below must be done while the ServiceTracker object is unlocked because of a
            possibility for reentry from customer code. Therefore, we have to swap d->listenerToken to a local variable
            and replace it with a default constructed ListenerToken object.
            */
            ListenerToken swappedToken;
            {
                auto l = d->Lock();
                US_UNUSED(l);
                std::swap(d->listenerToken, swappedToken);
            }
            try
            {
                d->context.RemoveListener(std::move(swappedToken));
            }
            catch (std::runtime_error const& /*e*/)
            {
                /* In case the context was stopped or invalid. */
            }

            std::shared_ptr<_TrackedService> outgoing = d->trackedService.Load();
            {
                auto l = d->Lock();
                US_UNUSED(l);

                if (outgoing == nullptr)
                {
                    return;
                }

                if (d->Tracked()->closed)
                {
                    return;
                }

                DIAG_LOG(*d->context.GetLogSink()) << "ServiceTracker<S,TTT>::close:" << d->filter;
                outgoing->Close();

                d->Modified();         /* clear the cache */
                outgoing->NotifyAll(); /* wake up any waiters */
            }

            try
            {
                outgoing->WaitOnCustomizersToFinish();
            }
            catch (std::exception const&)
            {
                // this can throw if the latch's count
                // is negative, which means the latch
                // cannot be used anymore. This can occur
                // when multiple threads are opening
                // and closing the same service tracker
                // repeatedly.
            }

            auto references = GetServiceReferences();
            for (auto& ref : references)
            {
                outgoing->Untrack(ref, ServiceEvent());
            }

            if (d->context.GetLogSink()->Enabled())
            {
                if (!d->cachedReference.Load().GetBundle() && d->cachedService.Load() == nullptr)
                {
                    DIAG_LOG(*d->context.GetLogSink()) << "ServiceTracker<S,TTT>::close[cached cleared]:" << d->filter;
                }
            }
        }

        /**
         * Wait for at least one service to be tracked by this
         * <code>ServiceTracker</code>. This method will also return when this
         * <code>ServiceTracker</code> is closed.
         *
         * <p>
         * It is strongly recommended that <code>WaitForService</code> is not used
         * during the calling of the <code>BundleActivator</code> methods.
         * <code>BundleActivator</code> methods are expected to complete in a short
         * period of time.
         *
         * <p>
         * This implementation calls GetService() to determine if a service
         * is being tracked.
         *
         * @return The result of GetService().
         */
        std::shared_ptr<TrackedParamType>
        WaitForService()
        {
            return WaitForService(std::chrono::milliseconds::zero());
        }

        /**
         * Wait for at least one service to be tracked by this
         * <code>ServiceTracker</code>. This method will also return when this
         * <code>ServiceTracker</code> is closed.
         *
         * <p>
         * It is strongly recommended that <code>WaitForService</code> is not used
         * during the calling of the <code>BundleActivator</code> methods.
         * <code>BundleActivator</code> methods are expected to complete in a short
         * period of time.
         *
         * <p>
         * This implementation calls GetService() to determine if a service
         * is being tracked.
         *
         * @param rel_time The relative time duration to wait for a service. If
         *        zero, the method will wait indefinitely.
         * @throws std::invalid_argument exception if \c rel_time is negative.
         * @return The result of GetService().
         */
        template <class Rep, class Period>
        std::shared_ptr<TrackedParamType>
        WaitForService(std::chrono::duration<Rep, Period> const& rel_time)
        {
            if (rel_time.count() < 0)
            {
                throw std::invalid_argument("negative timeout");
            }

            auto object = GetService();
            if (object)
            {
                return object;
            }

            using D = std::chrono::duration<Rep, Period>;

            auto timeout = rel_time;
            auto endTime = (rel_time == D::zero()) ? std::chrono::steady_clock::time_point()
                                                   : (std::chrono::steady_clock::now() + rel_time);
            do
            {
                auto t = d->Tracked();
                if (!t)
                { /* if ServiceTracker is not open */
                    return std::shared_ptr<TrackedParamType>();
                }

                {
                    auto l = t->Lock();
                    if (t->Size_unlocked() == 0)
                    {
                        t->WaitFor(l, rel_time, [&t] { return t->Size_unlocked() > 0 || t->closed; });
                    }
                }
                object = GetService();
                // Adapt the timeout in case we "missed" the object after having
                // been notified within the timeout.
                if (!object && endTime > std::chrono::steady_clock::time_point())
                {
                    timeout = std::chrono::duration_cast<D>(endTime - std::chrono::steady_clock::now());
                    if (timeout.count() <= 0)
                    {
                        break; // timed out
                    }
                }
            } while (!object && !d->Tracked()->closed);

            return object;
        }

        /**
         * Return a list of <code>ServiceReference</code>s for all services being
         * tracked by this <code>ServiceTracker</code>.
         *
         * @return A list of <code>ServiceReference</code> objects.
         */
        virtual std::vector<ServiceReference<S>>
        GetServiceReferences() const
        {
            std::vector<ServiceReference<S>> refs;
            auto t = d->Tracked();
            if (!t)
            { /* if ServiceTracker is not open */
                return refs;
            }
            {
                auto l = t->Lock();
                US_UNUSED(l);
                d->GetServiceReferences_unlocked(refs, t.get());
            }
            return refs;
        }

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
         * algorithm used by <code>BundleContext::GetServiceReference()</code>.
         *
         * <p>
         * This implementation calls GetServiceReferences() to get the list
         * of references for the tracked services.
         *
         * @return A <code>ServiceReference</code> for a tracked service.
         * @throws ServiceException if no services are being tracked.
         */
        virtual ServiceReference<S>
        GetServiceReference() const
        {
            ServiceReference<S> reference = d->cachedReference.Load();
            if (reference.GetBundle())
            {
                DIAG_LOG(*d->context.GetLogSink())
                    << "ServiceTracker<S,TTT>::getServiceReference[cached]:" << d->filter;
                return reference;
            }
            DIAG_LOG(*d->context.GetLogSink()) << "ServiceTracker<S,TTT>::getServiceReference:" << d->filter;
            auto references = GetServiceReferences();
            std::size_t length = references.size();
            if (length == 0)
            { /* if no service is being tracked */
                throw ServiceException("No service is being tracked");
            }
            auto selectedRef = references.begin();
            if (length > 1)
            { /* if more than one service, select highest ranking */
                std::vector<int> rankings(length);
                int count = 0;
                int maxRanking = (std::numeric_limits<int>::min)();
                auto refIter = references.begin();
                for (std::size_t i = 0; i < length; i++)
                {
                    Any rankingAny = refIter->GetProperty(Constants::SERVICE_RANKING);
                    int ranking = 0;
                    if (rankingAny.Type() == typeid(int))
                    {
                        ranking = any_cast<int>(rankingAny);
                    }

                    rankings[i] = ranking;
                    if (ranking > maxRanking)
                    {
                        selectedRef = refIter;
                        maxRanking = ranking;
                        count = 1;
                    }
                    else
                    {
                        if (ranking == maxRanking)
                        {
                            count++;
                        }
                    }
                    ++refIter;
                }
                if (count > 1)
                { /* if still more than one service, select lowest id */
                    long int minId = (std::numeric_limits<long int>::max)();
                    refIter = references.begin();
                    for (std::size_t i = 0; i < length; i++)
                    {
                        if (rankings[i] == maxRanking)
                        {
                            Any idAny = refIter->GetProperty(Constants::SERVICE_ID);
                            long int id = 0;
                            if (idAny.Type() == typeid(long int))
                            {
                                id = any_cast<long int>(idAny);
                            }
                            if (id < minId)
                            {
                                selectedRef = refIter;
                                minId = id;
                            }
                        }
                        ++refIter;
                    }
                }
            }

            d->cachedReference.Store(*selectedRef);
            return *selectedRef;
        }

        /**
         * Returns the service object for the specified
         * <code>ServiceReference</code> if the specified referenced service is
         * being tracked by this <code>ServiceTracker</code>.
         *
         * @param reference The reference to the desired service.
         * @return A service object or <code>nullptr</code> if the service referenced
         *         by the specified <code>ServiceReference</code> is not being
         *         tracked.
         */
        virtual std::shared_ptr<TrackedParamType>
        GetService(ServiceReference<S> const& reference) const
        {
            auto t = d->Tracked();
            if (!t)
            { /* if ServiceTracker is not open */
                return std::shared_ptr<TrackedParamType>();
            }
            return (t->Lock(), t->GetCustomizedObject_unlocked(reference));
        }

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
        virtual std::vector<std::shared_ptr<TrackedParamType>>
        GetServices() const
        {
            std::vector<std::shared_ptr<TrackedParamType>> services;
            auto t = d->Tracked();
            if (!t)
            { /* if ServiceTracker is not open */
                return services;
            }
            {
                auto l = t->Lock();
                US_UNUSED(l);
                std::vector<ServiceReference<S>> references;
                d->GetServiceReferences_unlocked(references, t.get());
                for (auto& ref : references)
                {
                    services.push_back(t->GetCustomizedObject_unlocked(ref));
                }
            }
            return services;
        }

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
        virtual std::shared_ptr<TrackedParamType>
        GetService() const
        {
            auto service = d->cachedService.Load();
            if (service)
            {
                DIAG_LOG(*d->context.GetLogSink()) << "ServiceTracker<S,TTT>::getService[cached]:" << d->filter;
                return service;
            }
            DIAG_LOG(*d->context.GetLogSink()) << "ServiceTracker<S,TTT>::getService:" << d->filter;

            try
            {
                auto reference = GetServiceReference();
                if (!reference.GetBundle())
                {
                    return std::shared_ptr<TrackedParamType>();
                }
                service = GetService(reference);
                d->cachedService.Store(service);
                return service;
            }
            catch (ServiceException const&)
            {
                return std::shared_ptr<TrackedParamType>();
            }
        }

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
        virtual void
        Remove(ServiceReference<S> const& reference)
        {
            auto t = d->Tracked();
            if (!t)
            { /* if ServiceTracker is not open */
                return;
            }
            t->Untrack(reference, ServiceEvent());
        }

        /**
         * Return the number of services being tracked by this
         * <code>ServiceTracker</code>.
         *
         * @return The number of services being tracked.
         */
        virtual int
        Size() const
        {
            auto t = d->Tracked();
            if (!t)
            { /* if ServiceTracker is not open */
                return 0;
            }
            return (t->Lock(), static_cast<int>(t->Size_unlocked()));
        }

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
        virtual int
        GetTrackingCount() const
        {
            auto t = d->Tracked();
            if (!t)
            { /* if ServiceTracker has not been opened */
                return -1;
            }
            {
                auto l = t->Lock();
                US_UNUSED(l);
                if (t->closed)
                { /* if ServiceTracker was closed */
                    return -1;
                }
                return t->GetTrackingCount();
            }
        }

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
        virtual void
        GetTracked(TrackingMap& map) const
        {
            auto t = d->Tracked();
            if (!t)
            { /* if ServiceTracker is not open */
                return;
            }
            t->Lock(), t->CopyEntries_unlocked(map);
        }

        /**
         * Return if this <code>ServiceTracker</code> is empty.
         *
         * @return <code>true</code> if this <code>ServiceTracker</code> is not tracking any
         *         services.
         */
        virtual bool
        IsEmpty() const
        {
            auto t = d->Tracked();
            if (!t)
            { /* if ServiceTracker is not open */
                return true;
            }
            return (t->Lock(), t->IsEmpty_unlocked());
        }

      protected:
        /**
         * Default implementation of the
         * <code>ServiceTrackerCustomizer::AddingService</code> method.
         *
         * <p>
         * This method is only called when this <code>ServiceTracker</code> has been
         * constructed with a <code>nullptr</code> ServiceTrackerCustomizer argument.
         *
         * <p>
         * This implementation returns the result of calling <code>GetService</code>
         * on the <code>BundleContext</code> with which this
         * <code>ServiceTracker</code> was created passing the specified
         * <code>ServiceReference</code>.
         * <p>
         * This method can be overridden in a subclass to customize the service
         * object to be tracked for the service being added. In that case, take care
         * not to rely on the default implementation of #RemovedService to unget the
         * service.
         *
         * @param reference The reference to the service being added to this
         *        <code>ServiceTracker</code>.
         * @return The service object to be tracked for the service added to this
         *         <code>ServiceTracker</code>.
         * @throws std::runtime_error If this BundleContext is no longer valid.
         * @throws std::invalid_argument If the specified
         *         <code>ServiceReference</code> is invalid (default constructed).
         * @see ServiceTrackerCustomizer::AddingService(const ServiceReference&)
         */
        std::shared_ptr<TrackedParamType>
        AddingService(ServiceReference<S> const& reference) override
        {
            return TypeTraits::ConvertToTrackedType(d->context.GetService(reference));
        }

        /**
         * Default implementation of the
         * <code>ServiceTrackerCustomizer::ModifiedService</code> method.
         *
         * <p>
         * This method is only called when this <code>ServiceTracker</code> has been
         * constructed with a <code>nullptr</code> ServiceTrackerCustomizer argument.
         *
         * <p>
         * This implementation does nothing.
         *
         * @param reference The reference to modified service.
         * @param service The service object for the modified service.
         * @see ServiceTrackerCustomizer::ModifiedService(const ServiceReference&, TrackedArgType)
         */
        void
        ModifiedService(ServiceReference<S> const& /*reference*/,
                        std::shared_ptr<TrackedParamType> const& /*service*/) override
        {
            /* do nothing */
        }

        /**
         * Default implementation of the
         * <code>ServiceTrackerCustomizer::RemovedService</code> method.
         *
         * <p>
         * This method is only called when this <code>ServiceTracker</code> has been
         * constructed with a <code>nullptr</code> ServiceTrackerCustomizer argument.
         *
         * This method can be overridden in a subclass. If the default
         * implementation of the #AddingService
         * method was used, this method must unget the service.
         *
         * @param reference The reference to removed service.
         * @param service The service object for the removed service.
         * @see ServiceTrackerCustomizer::RemovedService(const ServiceReferenceType&, TrackedArgType)
         */
        void
        RemovedService(ServiceReference<S> const& /*reference*/,
                       std::shared_ptr<TrackedParamType> const& /*service*/) override
        {
            /* do nothing */
        }

      private:
        using TypeTraits = typename ServiceTrackerCustomizer<S, T>::TypeTraits;

        using _ServiceTracker = ServiceTracker<S, T>;
        using _TrackedService = detail::TrackedService<S, TypeTraits>;
        using _ServiceTrackerPrivate = detail::ServiceTrackerPrivate<S, TypeTraits>;
        using _ServiceTrackerCustomizer = ServiceTrackerCustomizer<S, T>;

        friend class detail::TrackedService<S, TypeTraits>;
        friend class detail::ServiceTrackerPrivate<S, TypeTraits>;

        std::unique_ptr<_ServiceTrackerPrivate> d;
    };
} // namespace cppmicroservices

// include statements at end because of dependencies on ServiceTracker.h
#include "cppmicroservices/detail/ServiceTrackerPrivate.h"
#include "cppmicroservices/detail/TrackedService.h"

#endif // CPPMICROSERVICES_SERVICETRACKER_H
