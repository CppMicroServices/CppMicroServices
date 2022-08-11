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

#ifndef CPPMICROSERVICES_BUNDLETRACKERPRIVATE_H
#define CPPMICROSERVICES_BUNDLETRACKERPRIVATE_H

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/detail/Threads.h"

#include <memory>

namespace cppmicroservices {

namespace detail {

/**
 * \ingroup MicroServices
 */
template<class T>
class BundleTrackerPrivate : MultiThreaded<>
{

public:
  using BundleStateMaskType = std::underlying_type_t<Bundle::State>;

  BundleTrackerPrivate(
    BundleTracker<T>*,
    const BundleContext& _context,
    const BundleStateMaskType _stateMask,
    const std::shared_ptr<BundleTrackerCustomizer<T>> _customizer);
  ~BundleTrackerPrivate();

  /**
   * Returns the list of initial <code>Bundle</code>s that will be
   * tracked by the <code>BundleTracker</code>.
   * 
   * @param stateMask The mask of states to filter bundles
   * 
   * @return The list of initial <code>Bundle</code>s.
   */
  std::vector<Bundle> GetInitialBundles(BundleStateMaskType stateMask);

  void GetBundles_unlocked(std::vector<Bundle>& refs,
                           TrackedBundle<T>* t) const;

  /**
   * The Bundle Context used by this <code>BundleTracker</code>.
   */
  BundleContext context;

  /**
   * State mask for tracked bundles.
   */
  const BundleStateMaskType stateMask;

  /**
   * The <code>BundleTrackerCustomizer</code> for this tracker.
   */
  std::shared_ptr<BundleTrackerCustomizer<T>> customizer;

  /**
   * This token corresponds to the BundleListener, whenever it is added.
   * Otherwise, it represents an invalid token.
   */
  ListenerToken listenerToken;

  /**
   * Tracked bundles: <code>Bundle</code> -> custom Object
   */
  Atomic<std::shared_ptr<TrackedBundle<T>>> trackedBundle;

  /**
   * Accessor method for the current TrackedBundle object. This method is only
   * intended to be used by the unsynchronized methods which do not modify the
   * trackedBundle field.
   *
   * @return The current Tracked object.
   */
  std::shared_ptr<TrackedBundle<T>> Tracked() const;

  /**
   * Called by the TrackedBundle object whenever the set of tracked bundles is
   * modified.
   */
  /*
   * This method must not be synchronized since it is called by TrackedBundle while
   * TrackedBundle is synchronized. We don't want synchronization interactions
   * between the listener thread and the user thread.
   */
  void Modified();

private:
  inline BundleTrackerCustomizer<T>* getTrackerAsCustomizer()
  {
    return static_cast<BundleTrackerCustomizer<T>*>(bundleTracker);
  }

  inline const BundleTrackerCustomizer<T>* getTrackerAsCustomizer() const
  {
    return static_cast<BundleTrackerCustomizer<T>*>(bundleTracker);
  }

  friend class BundleTracker<T>;

  BundleTracker<T>* const bundleTracker;
};

} // namespace detail

} // namespace cppmicroservices

#include "cppmicroservices/detail/BundleTrackerPrivate.tpp"

#endif // CPPMICROSERVICES_BUNDLETRACKERPRIVATE_H
