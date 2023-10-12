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
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/detail/Threads.h"
#include "cppmicroservices/detail/TrackedBundle.h"

#include <memory>
#include <stdexcept>
#include <utility>

namespace cppmicroservices
{

    template <class T>
    class BundleTracker;

    namespace detail
    {

        /**
         * \ingroup MicroServices
         */
        template <class T>
        class BundleTrackerPrivate : MultiThreaded<>
        {

          public:
            using BundleStateMaskType = std::underlying_type_t<Bundle::State>;

            BundleTrackerPrivate(BundleTracker<T>* bundleTracker,
                                 BundleContext const& context,
                                 BundleStateMaskType const stateMask,
                                 std::shared_ptr<BundleTrackerCustomizer<T>> const customizer)
                : _context(context)
                , _stateMask(stateMask)
                , _customizer(customizer)
                , listenerToken()
                , trackedBundle()
                , _bundleTracker(bundleTracker)
            {
            }
            ~BundleTrackerPrivate() = default;

            /**
             * Returns the list of initial <code>Bundle</code>s that will be
             * tracked by the <code>BundleTracker</code>.
             *
             * @param stateMask The mask of states to filter bundles
             *
             * @return The list of initial <code>Bundle</code>s.
             */
            std::vector<Bundle>
            GetInitialBundles(BundleStateMaskType stateMask)
            {
                std::vector<Bundle> result;
                auto contextBundles = _context.GetBundles();
                for (Bundle bundle : contextBundles)
                {
                    if (bundle.GetState() & stateMask)
                    {
                        result.push_back(bundle);
                    }
                }
                return result;
            }

            void
            GetBundles_unlocked(std::vector<Bundle>& refs, TrackedBundle<T>* t) const
            {
                if (t->Size_unlocked() == 0)
                {
                    return;
                }
                t->GetTracked_unlocked(refs);
            }

            /**
             * The Bundle Context used by this <code>BundleTracker</code>.
             */
            BundleContext _context;

            /**
             * State mask for tracked bundles.
             */
            BundleStateMaskType const _stateMask;

            /**
             * The <code>BundleTrackerCustomizer</code> for this tracker.
             */
            std::shared_ptr<BundleTrackerCustomizer<T>> _customizer;

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
            std::shared_ptr<TrackedBundle<T>>
            Tracked() const
            {
                return trackedBundle.Load();
            }

            /**
             * Called by the TrackedBundle object whenever the set of tracked bundles is
             * modified.
             */
            void
            Modified()
            {
                // No cache to clear
            }

            inline BundleTrackerCustomizer<T>*
            GetCustomizer_unlocked()
            {
                return (_customizer ? _customizer.get() : static_cast<BundleTrackerCustomizer<T>*>(_bundleTracker));
            }

          private:
            friend class BundleTracker<T>;

            BundleTracker<T>* const _bundleTracker;
        };

    } // namespace detail

} // namespace cppmicroservices

#endif // CPPMICROSERVICES_BUNDLETRACKERPRIVATE_H
