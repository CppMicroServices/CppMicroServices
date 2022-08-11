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
  TrackedBundle(BundleTracker<T>* _bundleTracker,
                BundleTrackerCustomizer<T>* _customizer);

  /**
   * Method connected to bundle events for the
   * <code>BundleTracker</code> class. This method must NOT be
   * synchronized to avoid deadlock potential.
   *
   * @param event <code>BundleEvent</code> object from the framework.
   */
  void BundleChanged(const BundleEvent& event) override;

  void WaitOnCustomizersToFinish();

private:
  BundleTracker<T>* bundleTracker;
  BundleTrackerCustomizer<T>* customizer;

  CounterLatch latch;

  /**
   * Increment the tracking count and tell the tracker there was a
   * modification.
   *
   * @GuardedBy this
   */
  void Modified() override;

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
                                    const BundleEvent& related) override;

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
                          const T& object) override;

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
                         const T& object) override;
};

} // namespace detail

} // namespace cppmicroservices

#include "cppmicroservices/detail/TrackedBundle.tpp"

#endif // CPPMICROSERVICES_TRACKEDBUNDLE_H
