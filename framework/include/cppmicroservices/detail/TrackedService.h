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

#ifndef CPPMICROSERVICES_TRACKEDSERVICE_H
#define CPPMICROSERVICES_TRACKEDSERVICE_H

#include "cppmicroservices/ServiceEvent.h"
#include "cppmicroservices/detail/BundleAbstractTracked.h"
#include "cppmicroservices/detail/TrackedServiceListener.h"

#include "cppmicroservices/detail/CounterLatch.h"
#include "cppmicroservices/detail/ScopeGuard.h"

namespace cppmicroservices
{

    namespace detail
    {

        /**
         * This class is not intended to be used directly. It is exported to support
         * the CppMicroServices bundle system.
         */
        template <class S, class TTT>
        class TrackedService
            : public TrackedServiceListener
            , public BundleAbstractTracked<ServiceReference<S>, TTT, ServiceEvent>
        {

          public:
            using T = typename TTT::TrackedType;
            using TrackedParamType = typename TTT::TrackedParamType;

            TrackedService(ServiceTracker<S, T>* serviceTracker, ServiceTrackerCustomizer<S, T>* customizer)
                : Superclass(serviceTracker->d->context)
                , serviceTracker(serviceTracker)
                , customizer(customizer)
                , latch {}
            {
            }

            /**
             * Method connected to service events for the
             * <code>ServiceTracker</code> class. This method must NOT be
             * synchronized to avoid deadlock potential.
             *
             * @param event <code>ServiceEvent</code> object from the framework.
             */
            void
            ServiceChanged(ServiceEvent const& event) override
            {
                (void)latch.CountUp();
                ScopeGuard sg(
                    [this]()
                    {
                        // By using try/catch here, we ensure that this lambda function doesn't
                        // throw inside ScopeGuard's dtor.
                        try
                        {
                            latch.CountDown();
                        }
                        catch (...)
                        {
                        }
                    });

                ServiceReference<S> reference;
                {
                    auto l = this->Lock();
                    US_UNUSED(l);
                    /*
                     * Check if we had a delayed call (which could happen when we
                     * close).
                     */
                    if (this->closed)
                    {
                        return;
                    }

                    reference = event.GetServiceReference<S>();

                    DIAG_LOG(*serviceTracker->d->context.GetLogSink())
                        << "TrackedService::ServiceChanged[" << event.GetType() << "]: " << reference;
                    if (!reference)
                    {
                        return;
                    }
                }

                switch (event.GetType())
                {
                    case ServiceEvent::SERVICE_REGISTERED:
                    case ServiceEvent::SERVICE_MODIFIED:
                    {
                        if (!serviceTracker->d->listenerFilter.empty())
                        { // service listener added with filter
                            this->Track(reference, event);
                            /*
                             * If the customizer throws an unchecked exception, it
                             * is safe to let it propagate
                             */
                        }
                        else
                        { // service listener added without filter
                            if (serviceTracker->d->filter.Match(reference))
                            {
                                this->Track(reference, event);
                                /*
                                 * If the customizer throws an unchecked exception,
                                 * it is safe to let it propagate
                                 */
                            }
                            else
                            {
                                this->Untrack(reference, event);
                                /*
                                 * If the customizer throws an unchecked exception,
                                 * it is safe to let it propagate
                                 */
                            }
                        }
                        break;
                    }
                    case ServiceEvent::SERVICE_MODIFIED_ENDMATCH:
                    case ServiceEvent::SERVICE_UNREGISTERING:
                        this->Untrack(reference, event);
                        /*
                         * If the customizer throws an unchecked exception, it is
                         * safe to let it propagate
                         */
                        break;
                }
            }

            void
            WaitOnCustomizersToFinish()
            {
                latch.Wait();
            }

          private:
            using Superclass = BundleAbstractTracked<ServiceReference<S>, TTT, ServiceEvent>;

            ServiceTracker<S, T>* serviceTracker;
            ServiceTrackerCustomizer<S, T>* customizer;

            CounterLatch latch;

            /**
             * Increment the tracking count and tell the tracker there was a
             * modification.
             *
             * @GuardedBy this
             */
            void
            Modified() override
            {
                Superclass::Modified(); /* increment the modification count */
                serviceTracker->d->Modified();
            }

            /**
             * Call the specific customizer adding method. This method must not be
             * called while synchronized on this object.
             *
             * @param item Item to be tracked.
             * @param related Action related object.
             * @return Customized object for the tracked item or <code>null</code>
             *         if the item is not to be tracked.
             */
            std::shared_ptr<TrackedParamType>
            CustomizerAdding(ServiceReference<S> item, ServiceEvent const& /*related*/) override
            {
                return customizer->AddingService(item);
            }

            /**
             * Call the specific customizer modified method. This method must not be
             * called while synchronized on this object.
             *
             * @param item Tracked item.
             * @param related Action related object.
             * @param object Customized object for the tracked item.
             */
            void
            CustomizerModified(ServiceReference<S> item,
                               ServiceEvent const& /*related*/,
                               std::shared_ptr<TrackedParamType> const& object) override
            {
                customizer->ModifiedService(item, object);
            }

            /**
             * Call the specific customizer removed method. This method must not be
             * called while synchronized on this object.
             *
             * @param item Tracked item.
             * @param related Action related object.
             * @param object Customized object for the tracked item.
             */
            void
            CustomizerRemoved(ServiceReference<S> item,
                              ServiceEvent const& /*related*/,
                              std::shared_ptr<TrackedParamType> const& object) override
            {
                customizer->RemovedService(item, object);
            }
        };

    } // namespace detail

} // namespace cppmicroservices

#endif // CPPMICROSERVICES_TRACKEDSERVICE_H
