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

#ifndef CPPMICROSERVICES_BUNDLETRACKER_H
#define CPPMICROSERVICES_BUNDLETRACKER_H

#include <chrono>
#include <map>
#include <optional>
#include <type_traits>

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/BundleTrackerCustomizer.h"

namespace cppmicroservices {

namespace detail {
template<class T>
class TrackedBundle;
template<class T>
class BundleTrackerPrivate;
} // namespace detail

class BundleContext;

/**
\defgroup gr_bundletracker BundleTracker

\brief Groups BundleTracker related symbols.
*/

/**
 * \ingroup MicroServices
 * \ingroup gr_bundletracker
 * 
 * The <code>BundleTracker</code> class allows for keeping an accurate record of bundles and handling
 * state changes, including bundles started before the tracker is opened.
 *
 * <p>
 * A <code>BundleTracker</code> is constructed with a state mask and a <code>BundleTrackerCustomizer</code> object.
 * The <code>BundleTracker</code> can select certain bundles to be tracked and trigger custom callbacks
 * based on the provided <code>BundleTrackerCustomizer</code> or a <code>BundleTracker</code> subclass. Additionally,
 * the template parameter allows for the use of custom tracked objects. Once the <code>BundleTracker</code>
 * is opened, it will begin tracking all bundles that fall within the state mask.
 *
 * <p>
 * Use the <code>getBundles</code> method to get the tracked <code>Bundle</code> objects, and <code>getObject</code> 
 * to get the customized object.
 *
 * <p>
 * The <code>BundleTracker</code> class is thread-safe. It does not call <code>BundleTrackerCustomizer</code> methods while
 * holding any locks. Customizer implementations must be thread-safe.
 *
 * @tparam T The type of tracked object.
 * @remarks This class is thread safe.
 */
template<class T = Bundle>
class BundleTracker : protected BundleTrackerCustomizer<T>
{
public:
  // The type of tracked object
  using TrackedParamType =
    typename BundleTrackerCustomizer<T>::TrackedParamType;

  // The type of the tracking map
  using TrackingMap = typename std::unordered_map<Bundle, TrackedParamType>;

  // The type of the state mask
  using StateType = std::underlying_type_t<Bundle::State>;

  /**
   * Create a <code>BundleTracker</code> that tracks bundles through states covered by the state mask.
   * 
   * @param context The <code>BundleTrackerContext</code> from which tracking occurs.
   * @param stateMask The bit mask which defines the bundle states to be tracked.
   * @param customizer The customizer to call when bundles are added, modified, or removed.
   *                   If the customizer is nullptr, then the callbacks in this <code>BundleTracker</code> will 
   *                   be used instead (default or can be overridden).
   */
  BundleTracker(const BundleContext& context,
                StateType stateMask,
                BundleTrackerCustomizer<T>* customizer = nullptr);

  /**
   * Automatically closes the <code>BundleTracker</code>
   */
  ~BundleTracker() override;

  /**
   * Called when a <code>Bundle</code> is being added to the <code>BundleTracker</code> 
   * and the customizer constructor argument was nullptr.
   * 
   * When the <code>BundleTracker</code> detects a <code>Bundle</code> that should be added to the tracker 
   * based on the search parameters (state mask, context, etc.),
   * this method is called. This method should return the object to be tracked 
   * for the specified <code>Bundle</code> if the <code>BundleTracker</code> is being extended.
   * Otherwise, return the <code>Bundle</code> itself. If the return is nullptr, the <code>Bundle</code> is not tracked.
   *
   * @param bundle The <code>Bundle</code> being added to the <code>BundleTracker</code>.
   * @param event the BundleEvent which was caught by the <code>BundleTracker</code>.
   *
   * @return The object to be tracked for the specified <code>Bundle</code> object or nullptr to avoid tracking the Bundle.
   *
   * @see BundleTrackerCustomizer:AddingBundle(Bundle, BundleEvent)
   */
  std::optional<typename TrackedParamType> AddingBundle(
    const Bundle& bundle,
    const BundleEvent& event);

  /**
   * Close this <code>BundleTracker</code>.
   *
   * Removes all tracked bundles from this <code>BundleTracker</code>, calling <code>RemovedBundle</code>  on all of the
   * currently tracked bundles. Also resets the tracking count.
   */
  void Close();

  /**
   * Returns an array of all the tracked bundles.
   *
   * @return A vector of Bundles (could be empty).
   */
  std::vector<Bundle> GetBundles();

  /**
   * Returns the custom object for the given <code>Bundle</code> if the given <code>Bundle</code> is tracked. Otherwise null.
   *
   * @param bundle The <code>Bundle</code> paired with the object
   * @return The custom object paired with the given <code>Bundle</code> or null if the <code>Bundle</code> is not being tracked.
   */
  TrackedParamType GetObject(const Bundle& bundle);

  /**
   * Returns an unordered map from all of the currently tracked Bundles to their custom objects.
   *
   * @return An unordered map from all of the Bundles currently tracked by this
   * <code>BundleTracker</code> to their custom objects.
   */
  TrackingMap GetTracked();

  /**
   * Returns the tracking count for this <code>BundleTracker</code>.
   *
   * The tracking count is set to 0 when the <code>BundleTracker</code> is opened.
   * The tracking count increases by 1 anytime a <code>Bundle</code> is added,
   * modified, or removed from the <code>BundleTracker</code>.
   * Tracking counts from different times can be compared 
   * to determine whether any bundles have changed.
   * If the <code>BundleTracker</code> is closed, return -1.
   *
   * @return The current tracking count.
   */
  int GetTrackingCount();

  /**
   * Returns true if and only if this <code>BundleTracker</code> is tracking no bundles.
   *
   * @return true if and only if this <code>BundleTracker</code> is empty.
   */
  bool IsEmpty();

  /**
   * Called when a <code>Bundle</code> is modified that is being tracked by this <code>BundleTracker</code>
   * and the <code>BundleTrackerCustomizer</code> constructor argument was nullptr.
   *
   * When a tracked bundle changes states, this method is called.
   *
   * @param bundle The tracked <code>Bundle</code> whose state has changed.
   * @param event The BundleEvent which was caught by the <code>BundleTracker</code>. Can be null.
   * @param object The tracked object corresponding to the tracked <code>Bundle</code> (returned from AddingBundle).
   *
   * @see BundleTrackerCustomizer:ModifiedBundle(Bundle, BundleEvent, std::shared_ptr<T>)
   */
  void ModifiedBundle(const Bundle& bundle,
                      const BundleEvent& event,
                      TrackedParamType object);

  /**
   * Open this <code>BundleTracker</code> to begin tracking bundles.
   *
   * Bundles that match the state mask will be tracked by this <code>BundleTracker</code>.
   *
   * @throws std::logic_error If the <code>BundleTrackerContext</code> used in the creation of this
   * <code>BundleTracker</code> is no longer valid.
   */
  void Open();

  /**
   * Remove a bundle from this <code>BundleTracker</code>.
   *
   * @param bundle the <code>Bundle</code> to be removed
   */
  void Remove(const Bundle&);

  /**
   * Called when a <code>Bundle</code> is being removed from this <code>BundleTracker</code>
   * and the <code>BundleTrackerCustomizer</code> constructor argument was nullptr.
   *
   * @param bundle The tracked <code>Bundle</code> that is being removed.
   * @param event The BundleEvent which was caught by the <code>BundleTracker</code>. Can be null.
   * @param object The tracked object corresponding to the tracked <code>Bundle</code> (returned from AddingBundle).
   *
   * @see BundleTrackerCustomizer:RemovedBundle(Bundle, BundleEvent, std::shared_ptr<T>)
   */
  void RemovedBundle(const Bundle& bundle,
                     const BundleEvent& event,
                     TrackedParamType object);

  /**
   * Return the number of bundles being tracked by this <code>BundleTracker</code>.
   *
   * @return The number of tracked bundles.
   */
  size_t Size();

private:
  using TypeTraits = typename BundleTrackerCustomizer<T>::TypeTraits;
  using _BundleTracker = BundleTracker<T>;
  using _TrackedBundle = detail::TrackedBundle<TypeTraits>;
  using _BundleTrackerPrivate = detail::BundleTrackerPrivate<TypeTraits>;
  using _BundleTrackerCustomizer = BundleTrackerCustomizer<T>;
  using BundleState = std::underlying_type_t<typename BundleEvent::Type>;

  friend class detail::TrackedBundle<TypeTraits>;
  friend class detail::BundleTrackerPrivate<TypeTraits>;

  std::unique_ptr<_BundleTrackerPrivate> d;
};

} // namespace cppmicroservices

#include "cppmicroservices/detail/BundleTracker.tpp"

#endif // CPPMICROSERVICES_BUNDLETRACKER_H
