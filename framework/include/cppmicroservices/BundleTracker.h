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
 * holding any locks. <code>BundleTrackerCustomizer</code> implementations must be thread-safe.
 *
 * @tparam T The type of tracked object.
 * @remarks This class is thread safe.
 */
template<class T = Bundle>
class BundleTracker : protected BundleTrackerCustomizer<T>
{
public:
  // The type of the tracking map
  using TrackingMap = typename std::unordered_map<Bundle, T>;

  // The type of the state mask
  using BundleStateMaskType = std::underlying_type_t<Bundle::State>;

  /**
   * Create a <code>BundleStateMaskType</code> stateMask for a BundleTracker
   * 
   * @param s0 the <code>Bundle::State</code>.
   * @param s the subsequent <code>Bundle::States</code>.
   * 
   * @return The state mask.
   * 
   */
  template<typename S0, typename... S>
  static constexpr typename BundleTracker<T>::BundleStateMaskType
  CreateStateMask(S0 const& s0, S const&... s)
  {
    static_assert((std::is_enum_v<S0> && ... && std::is_enum_v<S>),
                  "The function requires enumerations.");
    static_assert((std::is_same_v<Bundle::State, std::decay_t<S0>> && ... &&
                   std::is_same_v<Bundle::State, std::decay_t<S>>),
                  "All values must be Bundle States.");
    return (s0 | ... | s);
  }

  /**
   * Create a <code>BundleTracker</code> that tracks bundles through states covered by the state mask.
   * 
   * @param context The <code>BundleContext</code> from which tracking occurs.
   * @param stateMask The bit mask which defines the bundle states to be tracked.
   * @param customizer The customizer to call when bundles are added, modified, or removed.
   *                   If the customizer is nullptr, then the callbacks in this <code>BundleTracker</code> will 
   *                   be used instead (default or can be overridden).
   */
  BundleTracker(
    const BundleContext& context,
    const BundleStateMaskType stateMask,
    const std::shared_ptr<BundleTrackerCustomizer<T>> customizer = nullptr);

  /**
   * Automatically closes the <code>BundleTracker</code>
   */
  ~BundleTracker() override;

  /**
   * Close this <code>BundleTracker</code>.
   *
   * Removes all tracked bundles from this <code>BundleTracker</code>, calling <code>RemovedBundle</code>  on all of the
   * currently tracked bundles. Also resets the tracking count.
   */
  void Close() noexcept;

  /**
   * Returns a vector of all the tracked bundles.
   *
   * @return A vector of Bundles (could be empty).
   */
  std::vector<Bundle> GetBundles() const noexcept;

  /**
   * Returns the custom object for the given <code>Bundle</code> if the given <code>Bundle</code> is tracked. Otherwise nullopt.
   *
   * @param bundle The <code>Bundle</code> paired with the object
   * @return The custom object paired with the given <code>Bundle</code> or nullopt if the <code>Bundle</code> is not being tracked.
   */
  std::optional<T> GetObject(const Bundle& bundle) const noexcept;

  /**
   * Returns an unordered map containing all of the currently tracked Bundles to their custom objects.
   *
   * @return An unordered map containing all of the Bundles currently tracked by this
   * <code>BundleTracker</code> to their custom objects.
   */
  TrackingMap GetTracked() const noexcept;

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
   * @return The current tracking count, or -1 if the <code>BundleTracker</code> is closed.
   */
  int GetTrackingCount() const noexcept;

  /**
   * Returns true if and only if this <code>BundleTracker</code> is tracking no bundles.
   *
   * @return true if and only if this <code>BundleTracker</code> is tracking no bundles.
   */
  bool IsEmpty() const noexcept;

  /**
   * Open this <code>BundleTracker</code> to begin tracking bundles.
   *
   * Bundles that match the state mask will be tracked by this <code>BundleTracker</code>.
   *
   * @throws std::logic_error If the <code>BundleContext</code> used in the creation of this
   * <code>BundleTracker</code> is no longer valid.
   */
  void Open();

  /**
   * Remove a bundle from this <code>BundleTracker</code>.
   * 
   * @param bundle the <code>Bundle</code> to be removed
   * 
   * @see BundleTrackerCustomizer:RemovedBundle(Bundle, BundleEvent, T)
   */
  void Remove(const Bundle&) noexcept;

  /**
   * Return the number of bundles being tracked by this <code>BundleTracker</code>.
   *
   * @return The number of tracked bundles.
   */
  size_t Size() const noexcept;

  /**
   * Called when a <code>Bundle</code> is being added to the <code>BundleTracker</code> 
   * 
   * When a <code>Bundle</code> enters a state covered by the <code>BundleTracker</code>'s state mask
   * and the <code>Bundle</code> is not currently tracked, this method is called.
   * This method is also called if the <code>Bundle</code>'s state is covered by the state mask when 
   * the <code>BundleTracker</code> is opened.
   * 
   * This method should return the object to be tracked for the specified <code>Bundle</code>
   * if the <code>BundleTracker</code> is being extended.
   * Otherwise, return the <code>Bundle</code> itself. If the return is nullopt, the Bundle is not tracked.
   * 
   * The default, uncustomized behavior is to track the Bundle.
   *
   * @param bundle The <code>Bundle</code> being added to the <code>BundleTracker</code>.
   * @param event the BundleEvent which was caught by the <code>BundleTracker</code>.
   *
   * @return The object to be tracked for the specified <code>Bundle</code> object or nullptr to avoid tracking the Bundle.
   *
   * @see BundleTrackerCustomizer::AddingBundle(Bundle, BundleEvent)
   */
  virtual std::optional<T> AddingBundle(const Bundle& bundle,
                                        const BundleEvent& event) override;

  /**
   * Called when a <code>Bundle</code> is modified that is being tracked by this <code>BundleTracker</code>.
   *
   * When a <code>Bundle</code> enters a state covered by the <code>BundleTracker</code>'s state mask
   * and the <code>Bundle</code> is currently tracked, this method is called.
   * 
   * The default, uncustomized behavior is to no-op.
   * 
   * @param bundle The tracked <code>Bundle</code> whose state has changed.
   * @param event The BundleEvent which was caught by the <code>BundleTracker</code>.
   * @param object The tracked object corresponding to the tracked <code>Bundle</code>.
   *
   * @see BundleTrackerCustomizer:ModifiedBundle(Bundle, BundleEvent, T)
   */
  virtual void ModifiedBundle(const Bundle& bundle,
                              const BundleEvent& event,
                              const T& object) override;

  /**
   * Called when a <code>Bundle</code> is removed that is being tracked by this <code>BundleTracker</code>.
   * 
   * When a <code>Bundle</code> enters a state not covered by the <code>BundleTracker</code>'s state mask
   * and the <code>Bundle</code> is curently tracked, this method is called.
   * This method is also called if the <code>Bundle</code> is currently being tracked when
   * the <code>BundleTracker</code> is closed, or if Remove(Bundle) is called.
   * 
   * The default, uncustomized behavior is to no-op.
   *
   * @param bundle The tracked <code>Bundle</code> that is being removed.
   * @param event The BundleEvent which was caught by the <code>BundleTracker</code>. Can be null.
   * @param object The tracked object corresponding to the tracked <code>Bundle</code>.
   *
   * @see BundleTrackerCustomizer:RemovedBundle(Bundle, BundleEvent, T)
   */
  virtual void RemovedBundle(const Bundle& bundle,
                             const BundleEvent& event,
                             const T& object) override;

private:
  friend class detail::TrackedBundle<T>;
  friend class detail::BundleTrackerPrivate<T>;

  std::unique_ptr<detail::BundleTrackerPrivate<T>> d;
};

} // namespace cppmicroservices

#include "cppmicroservices/detail/BundleTracker.tpp"

#endif // CPPMICROSERVICES_BUNDLETRACKER_H
