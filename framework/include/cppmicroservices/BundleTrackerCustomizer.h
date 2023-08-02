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

#ifndef CPPMICROSERVICES_BUNDLETRACKERCUSTOMIZER_H
#define CPPMICROSERVICES_BUNDLETRACKERCUSTOMIZER_H

#include "cppmicroservices/Bundle.h"
#include <optional>
#include <type_traits>

namespace cppmicroservices
{

    /**
     * \ingroup MicroServices
     * \ingroup gr_bundletracker
     *
     * The <code>BundleTrackerCustomizer</code> interface allows for user callbacks to be included in a
     * <code>BundleTracker</code>. These callback methods can customize the objects that are tracked,
     * or trigger other behavior.
     * A <code>BundleTrackerCustomizer</code> is called when a <code>Bundle</code> is being added to a
     * <code>BundleTracker</code>, and it can then return an object for that tracked bundle. A
     * <code>BundleTrackerCustomizer</code>, is also called when a tracked bundle is modified or has been removed from a
     * <code>BundleTracker</code>.
     *
     * <p>
     * Bundle events are received synchronously by the <code>BundleTracker</code>, so it is recommended that
     * implementations of the <code>BundleTrackerCustomizer</code> do not alter bundle states while being synchronized
     * on any object.
     *
     * @tparam T The type of the tracked object. Defaults to <code>Bundle</code>.
     * @remarks This class is thread safe. All implementations should also be thread safe.
     */
    template <class T = Bundle>
    struct BundleTrackerCustomizer
    {

        static T
        ConvertToTrackedType(Bundle const& b)
        {
            if constexpr (std::is_same_v<T, Bundle>)
            {
                return b;
            }
            else
            {
                throw std::runtime_error("A custom BundleTrackerCustomizer instance is "
                                         "required for custom tracked objects.");
            }
        }

        virtual ~BundleTrackerCustomizer() = default;

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
         * Otherwise, return the <code>Bundle</code> itself. If the return is std::nullopt, the Bundle is not tracked.
         *
         * @param bundle The <code>Bundle</code> being added to the <code>BundleTracker</code>.
         * @param event the <code>BundleEvent</code> which was caught by the <code>BundleTracker</code>.
         *
         * @return The object to be tracked for the specified <code>Bundle</code> object or std::nullopt to avoid
         * tracking the <code>Bundle</code>.
         */
        virtual std::optional<T> AddingBundle(Bundle const& bundle, BundleEvent const& event) = 0;

        /**
         * Called when a <code>Bundle</code> is modified that is being tracked by this <code>BundleTracker</code>.
         *
         * When a <code>Bundle</code> enters a state covered by the <code>BundleTracker</code>'s state mask
         * and the <code>Bundle</code> is currently tracked, this method is called.
         *
         * @param bundle The tracked <code>Bundle</code> whose state has changed.
         * @param event The <code>BundleEvent</code> which was caught by the <code>BundleTracker</code>.
         * @param object The tracked object corresponding to the tracked <code>Bundle</code>.
         */
        virtual void ModifiedBundle(Bundle const& bundle, BundleEvent const& event, T const& object) = 0;

        /**
         * Called when a <code>Bundle</code> is removed that is being tracked by this <code>BundleTracker</code>.
         *
         * When a <code>Bundle</code> enters a state not covered by the <code>BundleTracker</code>'s state mask
         * and the <code>Bundle</code> is curently tracked, this method is called.
         * This method is also called if the <code>Bundle</code> is currently being tracked when
         * the <code>BundleTracker</code> is closed, or if Remove(Bundle) is called.
         *
         * @param bundle The tracked <code>Bundle</code> whose state has changed.
         * @param event The <code>BundleEvent</code> which was caught by the <code>BundleTracker</code>. Can be null.
         * @param object The tracked object corresponding to the tracked <code>Bundle</code>
         */
        virtual void RemovedBundle(Bundle const& bundle, BundleEvent const& event, T const& object) = 0;
    };

} // namespace cppmicroservices

#endif // CPPMICROSERVICES_BUNDLETRACKERCUSTOMIZER_H
