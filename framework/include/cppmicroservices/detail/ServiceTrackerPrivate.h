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

#ifndef CPPMICROSERVICES_SERVICETRACKERPRIVATE_H
#define CPPMICROSERVICES_SERVICETRACKERPRIVATE_H

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/LDAPFilter.h"
#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/detail/Threads.h"
#include "cppmicroservices/detail/TrackedService.h"

#include "cppmicroservices/Constants.h"

#include <stdexcept>
#include <utility>

namespace cppmicroservices
{

    namespace detail
    {

        /**
         * \ingroup MicroServices
         */
        template <class S, class TTT>
        class ServiceTrackerPrivate : MultiThreaded<>
        {

          public:
            using T = typename TTT::TrackedType;
            using TrackedParamType = typename TTT::TrackedParamType;

            ServiceTrackerPrivate(ServiceTracker<S, T>* st,
                                  BundleContext context,
                                  ServiceReference<S> const& reference,
                                  ServiceTrackerCustomizer<S, T>* customizer)
                : context(std::move(context))
                , customizer(customizer)
                , listenerToken()
                , trackReference(reference)
                , trackedService()
                , cachedReference()
                , cachedService()
                , q_ptr(st)
            {
                this->customizer = customizer ? customizer : q_func();
                std::stringstream ss;
                ss << "(" << Constants::SERVICE_ID << "="
                   << any_cast<long>(reference.GetProperty(Constants::SERVICE_ID)) << ")";
                this->listenerFilter = ss.str();
                try
                {
                    this->filter = LDAPFilter(listenerFilter);
                }
                catch (std::invalid_argument const& e)
                {
                    /*
                     * we could only get this exception if the ServiceReference was
                     * invalid
                     */
                    std::invalid_argument ia(std::string("unexpected std::invalid_argument exception: ") + e.what());
                    throw ia;
                }
            }

            ServiceTrackerPrivate(ServiceTracker<S, T>* st,
                                  BundleContext context,
                                  std::string const& clazz,
                                  ServiceTrackerCustomizer<S, T>* customizer)
                : context(std::move(context))
                , customizer(customizer)
                , listenerToken()
                , trackClass(clazz)
                , trackReference()
                , trackedService()
                , cachedReference()
                , cachedService()
                , q_ptr(st)
            {
                this->customizer = customizer ? customizer : q_func();
                this->listenerFilter = std::string("(") + cppmicroservices::Constants::OBJECTCLASS + "=" + clazz + ")";
                try
                {
                    this->filter = LDAPFilter(listenerFilter);
                }
                catch (std::invalid_argument const& e)
                {
                    /*
                     * we could only get this exception if the clazz argument was
                     * malformed
                     */
                    std::invalid_argument ia(std::string("unexpected std::invalid_argument exception: ") + e.what());
                    throw ia;
                }
            }

            ServiceTrackerPrivate(ServiceTracker<S, T>* st,
                                  BundleContext const& context,
                                  LDAPFilter const& filter,
                                  ServiceTrackerCustomizer<S, T>* customizer)
                : context(context)
                , filter(filter)
                , customizer(customizer)
                , listenerFilter(filter.ToString())
                , listenerToken()
                , trackReference()
                , trackedService()
                , cachedReference()
                , cachedService()
                , q_ptr(st)
            {
                this->customizer = customizer ? customizer : q_func();
                if (!context)
                {
                    throw std::invalid_argument("The bundle context cannot be null.");
                }
            }

            ~ServiceTrackerPrivate() = default;

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
            std::vector<ServiceReference<S>>
            GetInitialReferences(std::string const& className, std::string const& filterString)
            {
                std::vector<ServiceReference<S>> result;
                std::vector<ServiceReferenceU> refs = context.GetServiceReferences(className, filterString);
                for (std::vector<ServiceReferenceU>::const_iterator iter = refs.begin(); iter != refs.end(); ++iter)
                {
                    ServiceReference<S> ref(*iter);
                    if (ref)
                    {
                        result.push_back(ref);
                    }
                }
                return result;
            }

            void
            GetServiceReferences_unlocked(std::vector<ServiceReference<S>>& refs, TrackedService<S, TTT>* t) const
            {
                if (t->Size_unlocked() == 0)
                {
                    return;
                }
                t->GetTracked_unlocked(refs);
            }

            /**
             * The Bundle Context used by this <code>ServiceTracker</code>.
             */
            BundleContext context;

            /**
             * The filter used by this <code>ServiceTracker</code> which specifies the
             * search criteria for the services to track.
             */
            LDAPFilter filter;

            /**
             * The <code>ServiceTrackerCustomizer</code> for this tracker.
             */
            ServiceTrackerCustomizer<S, T>* customizer;

            /**
             * Filter string for use when adding the ServiceListener. If this field is
             * set, then certain optimizations can be taken since we don't have a user
             * supplied filter.
             */
            std::string listenerFilter;

            /**
             * This token corresponds to the ServiceListener, whenever it is added.
             * Otherwise, it represents an invalid token.
             */
            ListenerToken listenerToken;

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
            Atomic<std::shared_ptr<TrackedService<S, TTT>>> trackedService;

            /**
             * Accessor method for the current TrackedService object. This method is only
             * intended to be used by the unsynchronized methods which do not modify the
             * trackedService field.
             *
             * @return The current Tracked object.
             */
            std::shared_ptr<TrackedService<S, TTT>>
            Tracked() const
            {
                return trackedService.Load();
            }

            /**
             * Called by the TrackedService object whenever the set of tracked services is
             * modified. Clears the cache.
             */
            /*
             * This method must not be synchronized since it is called by TrackedService while
             * TrackedService is synchronized. We don't want synchronization interactions
             * between the listener thread and the user thread.
             */
            void
            Modified()
            {
                cachedReference.Store(ServiceReference<S>());             /* clear cached value */
                cachedService.Store(std::shared_ptr<TrackedParamType>()); /* clear cached value */
                DIAG_LOG(*context.GetLogSink()) << "ServiceTracker::Modified(): " << filter;
            }

            /**
             * Cached ServiceReference for getServiceReference.
             */
            mutable Atomic<ServiceReference<S>> cachedReference;

            /**
             * Cached service object for GetService.
             */
            mutable Atomic<std::shared_ptr<TrackedParamType>> cachedService;

          private:
            inline ServiceTracker<S, T>*
            q_func()
            {
                return static_cast<ServiceTracker<S, T>*>(q_ptr);
            }

            inline ServiceTracker<S, T> const*
            q_func() const
            {
                return static_cast<ServiceTracker<S, T> const*>(q_ptr);
            }

            friend class ServiceTracker<S, T>;

            ServiceTracker<S, T>* const q_ptr;
        };

    } // namespace detail

} // namespace cppmicroservices

#endif // CPPMICROSERVICES_SERVICETRACKERPRIVATE_H
