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

#ifndef CPPMICROSERVICES_SERVICETRACKERCUSTOMIZER_H
#define CPPMICROSERVICES_SERVICETRACKERCUSTOMIZER_H

#include "cppmicroservices/ServiceReference.h"

namespace cppmicroservices
{

    /**
     * \ingroup MicroServices
     * \ingroup gr_servicetracker
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
     * The methods in this interface may be called as the result of a
     * <code>ServiceEvent</code> being received by a <code>ServiceTracker</code>.
     * Since <code>ServiceEvent</code>s are synchronously delivered,
     * it is highly recommended that implementations of these methods do
     * not register (<code>BundleContext::RegisterService</code>), modify (
     * <code>ServiceRegistration::SetProperties</code>) or unregister (
     * <code>ServiceRegistration::Unregister</code>) a service while being
     * synchronized on any object.
     *
     * \note The <code>ServiceTracker</code> class implementation of <code>ServiceTrackerCustomizer</code>
     *       is thread-safe. It does not call a <code>ServiceTrackerCustomizer</code> while holding any locks.
     *       <code>ServiceTrackerCustomizer</code> implementations must also be thread-safe.
     *
     * \tparam S The type of the service being tracked
     * \tparam T The type of the tracked object. The default is \c S.
     * \remarks <code>ServiceTrackerCustomizer</code> implementations must also be thread-safe.
     */
    template <class S, class T = S>
    struct ServiceTrackerCustomizer
    {

        struct TypeTraits
        {
            using ServiceType = S;
            using TrackedType = T;
            using TrackedParamType = T;

            static std::shared_ptr<TrackedType>
            ConvertToTrackedType(std::shared_ptr<S> const&)
            {
                throw std::runtime_error("A custom ServiceTrackerCustomizer instance is "
                                         "required for custom tracked objects.");
            }
        };

        using TrackedParamType = typename TypeTraits::TrackedParamType;

        virtual ~ServiceTrackerCustomizer() = default;

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
        virtual std::shared_ptr<TrackedParamType> AddingService(ServiceReference<S> const& reference) = 0;

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
        virtual void ModifiedService(ServiceReference<S> const& reference,
                                     std::shared_ptr<TrackedParamType> const& service)
            = 0;

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
        virtual void RemovedService(ServiceReference<S> const& reference,
                                    std::shared_ptr<TrackedParamType> const& service)
            = 0;
    };

    template <class S>
    struct ServiceTrackerCustomizer<S, S>
    {

        struct TypeTraits
        {
            using ServiceType = S;
            using TrackedType = S;
            using TrackedParamType = S;

            static std::shared_ptr<S>
            ConvertToTrackedType(std::shared_ptr<S> const& t)
            {
                return t;
            }
        };

        using TrackedParamType = typename TypeTraits::TrackedParamType;

        virtual ~ServiceTrackerCustomizer() = default;

        virtual std::shared_ptr<TrackedParamType> AddingService(ServiceReference<S> const& reference) = 0;
        virtual void ModifiedService(ServiceReference<S> const& reference,
                                     std::shared_ptr<TrackedParamType> const& service)
            = 0;
        virtual void RemovedService(ServiceReference<S> const& reference,
                                    std::shared_ptr<TrackedParamType> const& service)
            = 0;
    };

    template <class T>
    struct ServiceTrackerCustomizer<void, T>
    {

        struct TypeTraits
        {
            using ServiceType = void;
            using TrackedType = T;
            using TrackedParamType = T;

            static std::shared_ptr<T>
            ConvertToTrackedType(InterfaceMapConstPtr const&)
            {
                throw std::runtime_error("A custom ServiceTrackerCustomizer instance is "
                                         "required for custom tracked objects.");
            }
        };

        using S = void;
        using TrackedParamType = typename TypeTraits::TrackedParamType;

        virtual ~ServiceTrackerCustomizer() = default;
        virtual std::shared_ptr<TrackedParamType> AddingService(ServiceReference<S> const& reference) = 0;
        virtual void ModifiedService(ServiceReference<S> const& reference,
                                     std::shared_ptr<TrackedParamType> const& service)
            = 0;
        virtual void RemovedService(ServiceReference<S> const& reference,
                                    std::shared_ptr<TrackedParamType> const& service)
            = 0;
    };

    template <>
    struct ServiceTrackerCustomizer<void, void>
    {

        struct TypeTraits
        {
            using ServiceType = void;
            using TrackedType = void;
            using TrackedParamType = const InterfaceMap;

            static std::shared_ptr<TrackedParamType>
            ConvertToTrackedType(std::shared_ptr<TrackedParamType> const& t)
            {
                return t;
            }
        };

        using S = void;
        using T = void;
        using TrackedParamType = TypeTraits::TrackedParamType;

        virtual ~ServiceTrackerCustomizer() = default;
        virtual std::shared_ptr<TrackedParamType> AddingService(ServiceReference<S> const& reference) = 0;
        virtual void ModifiedService(ServiceReference<S> const& reference,
                                     std::shared_ptr<TrackedParamType> const& service)
            = 0;
        virtual void RemovedService(ServiceReference<S> const& reference,
                                    std::shared_ptr<TrackedParamType> const& service)
            = 0;
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_SERVICETRACKERCUSTOMIZER_H
