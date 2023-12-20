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

#ifndef CPPMICROSERVICES_TRACKEDBUNDLE_H
#define CPPMICROSERVICES_TRACKEDBUNDLE_H

#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/detail/BundleAbstractTracked.h"
#include "cppmicroservices/detail/TrackedBundleListener.h"

#include "cppmicroservices/detail/CounterLatch.h"
#include "cppmicroservices/detail/ScopeGuard.h"

namespace cppmicroservices {

template<class T>
class BundleTracker;

namespace detail {

/**
 * This class is not intended to be used directly. It is exported to support
 * the CppMicroServices bundle system.
 */
template<class T = Bundle>
class TrackedBundle
  : public TrackedBundleListener
  , public BundleAbstractTracked<Bundle, T, BundleEvent>
{

public:
  TrackedBundle(BundleTracker<T>* bundleTracker,
                BundleTrackerCustomizer<T>* customizer)
    : BundleAbstractTracked<Bundle, T, BundleEvent>(bundleTracker->d->_context)
    , _bundleTracker(bundleTracker)
    , _customizer(customizer)
    , latch{}
  {}

  /**
   * Method connected to bundle events for the
   * <code>BundleTracker</code> class. This method must NOT be
   * synchronized to avoid deadlock potential.
   *
   * @param event <code>BundleEvent</code> object from the framework.
   */
  void BundleChanged(const BundleEvent& event) override
  {
    // Call track or untrack based on state mask

    (void)latch.CountUp();
    ScopeGuard sg([this]() {
      // By using try/catch here, we ensure that this lambda function doesn't
      // throw inside ScopeGuard
      try {
        latch.CountDown();
      } catch (...) {
      }
    });

    // Check trivial calls
    Bundle bundle = event.GetBundle();
    if (!bundle) {
      return;
    }
    Bundle::State state = bundle.GetState();
    if (!state) {
      return;
    }
    // Ignore events that do not correspond with
    // Bundle state changes
    BundleEvent::Type eventType = event.GetType();
    if (eventType == BundleEvent::Type::BUNDLE_UNRESOLVED) {
      return;
    }
    {
      auto l = this->Lock();
      US_UNUSED(l);

      // Check for delayed call
      if (this->closed) {
        return;
      }
    }

    // Track iff state in mask
    if (state & _bundleTracker->d->_stateMask) {
      /*
     * The below method will throw if a customizer throws,
     * and the exception will propagate to the listener.
     */
      this->Track(bundle, event);
    } else {
      /*
     * The below method will throw if a customizer throws,
     * and the exception will propagate to the listener.
     */
      this->Untrack(bundle, event);
    }
  }

  void WaitOnCustomizersToFinish() { latch.Wait(); }

private:
  BundleTracker<T>* _bundleTracker;
  BundleTrackerCustomizer<T>* _customizer;

  CounterLatch latch;

  /**
   * Increment the tracking count and tell the tracker there was a
   * modification.
   *
   * @GuardedBy this
   */
  void Modified() override
  {
    BundleAbstractTracked<Bundle, T, BundleEvent>::
      Modified(); /* increment the modification count */
    _bundleTracker->d->Modified();
  }

  /**
   * Call the specific customizer adding method. This method must not be
   * called while synchronized on this object.
   *
   * @param bundle Bundle to be tracked.
   * @param related Action related object.
   * @return Customized object for the tracked bundle or <code>null</code>
   *         if the bundle is not to be tracked.
   * 
   * @see BundleTrackerCustomizer::AddingBundle(Bundle, BundleEvent)
   */
  std::optional<T> CustomizerAdding(Bundle bundle,
                                    const BundleEvent& related) override
  {
    return _customizer->AddingBundle(bundle, related);
  }

  /**
   * Call the specific customizer modified method. This method must not be
   * called while synchronized on this object.
   *
   * @param bundle Tracked bundle.
   * @param related Action related object.
   * @param object Customized object for the tracked bundle.
   * 
   * @see BundleTrackerCustomizer::ModifiedBundle(Bundle, BundleEvent, T)
   */
  void CustomizerModified(Bundle bundle,
                          const BundleEvent& related,
                          const T& object) override
  {
    _customizer->ModifiedBundle(bundle, related, object);
  }

  /**
   * Call the specific customizer removed method. This method must not be
   * called while synchronized on this object.
   *
   * @param bundle Tracked bundle.
   * @param related Action related object.
   * @param object Customized object for the tracked bundle.
   * 
   * @see BundleTrackerCustomizer::RemovedBundle(Bundle, BundleEvent, T)
   */
  void CustomizerRemoved(Bundle bundle,
                         const BundleEvent& related,
                         const T& object) override
  {
    _customizer->RemovedBundle(bundle, related, object);
  }
};

} // namespace detail

} // namespace cppmicroservices

#endif // CPPMICROSERVICES_TRACKEDBUNDLE_H
