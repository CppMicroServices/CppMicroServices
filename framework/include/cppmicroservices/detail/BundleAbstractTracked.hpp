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

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/detail/Log.h"
#include "cppmicroservices/detail/ScopeGuard.h"

#include <iterator>

namespace cppmicroservices
{

    namespace detail
    {

        template <class S, class T, class R>
        BundleAbstractTracked<S, T, R>::BundleAbstractTracked(BundleContext context)
            : closed(false)
            , trackingCount(0)
            , bc(std::move(context))
        {
        }

        template <class S, class T, class R>
        BundleAbstractTracked<S, T, R>::~BundleAbstractTracked() = default;

        template <class S, class T, class R>
        void
        BundleAbstractTracked<S, T, R>::SetInitial(std::vector<S> const& initiallist)
        {
            std::copy(initiallist.begin(), initiallist.end(), std::back_inserter(initial));
        }

        template <class S, class T, class R>
        void
        BundleAbstractTracked<S, T, R>::TrackInitial()
        {
            detail::ScopeGuard cleanupFutures(
                [this]()
                {
                    auto l = this->Lock();
                    US_UNUSED(l);
                    previouslyAdded.clear();
                    trackInitialFinished = true;
                });
            while (true)
            {
                S item;
                {
                    auto l = this->Lock();
                    US_UNUSED(l);
                    if (closed || (initial.size() == 0))
                    {
                        /*
                         * if there are no more initial items
                         */
                        return; /* we are done */
                    }
                    /*
                     * move the first item from the initial list to the adding list
                     * within this synchronized block.
                     */
                    item = initial.front();
                    initial.pop_front();
                    if (previouslyAdded.find(item) != previouslyAdded.end())
                    {
                        /* if we have already notified customizer for this bundle,
                        that was done through an event which takes precedence over a trackInitial call */
                        continue; /* skip this item */
                    }
                    if (std::find(adding.begin(), adding.end(), item) != adding.end())
                    {
                        /*
                         * if this item is already in the process of being added.
                         */
                        continue; /* skip this item */
                    }
                    adding.push_back(item);
                }
                TrackAdding(item, R());
                /*
                 * Begin tracking it. We call trackAdding
                 * since we have already put the item in the
                 * adding list.
                 */
            }
        }

        template <class S, class T, class R>
        void
        BundleAbstractTracked<S, T, R>::Close()
        {
            closed = true;
        }

        template <class S, class T, class R>
        void
        BundleAbstractTracked<S, T, R>::Track(S item, R related)
        {
            bool isInMap = false;
            T object;
            {
                auto l = this->Lock();
                US_UNUSED(l);
                if (closed)
                {
                    return;
                }
                auto trackedItemIter = tracked.find(item);
                if (trackedItemIter != tracked.end())
                {
                    object = trackedItemIter->second;
                    isInMap = true;
                }
                if (!isInMap)
                { /* we are not tracking the item */
                    if (std::find(adding.begin(), adding.end(), item) != adding.end())
                    {
                        /* if this item is already in the process of being added. */
                        return;
                    }
                    adding.push_back(item); /* mark this item is being added */
                    if (!trackInitialFinished)
                    {
                        previouslyAdded.insert(item);
                    }
                }
                else
                {               /* we are currently tracking this item */
                    Modified(); /* increment modification count */
                }
            }

            if (!isInMap)
            { /* we are not tracking the item */
                TrackAdding(item, related);
            }
            else
            {
                /* Call customizer outside of synchronized region */
                CustomizerModified(item, related, object);
                /*
                 * If the customizer throws an unchecked exception, it is safe to
                 * let it propagate
                 */
            }
        }

        template <class S, class T, class R>
        void
        BundleAbstractTracked<S, T, R>::Untrack(S item, R related)
        {
            T object;
            {
                auto l = this->Lock();
                US_UNUSED(l);
                std::size_t initialSize = initial.size();
                initial.remove(item);
                if (initialSize != initial.size())
                {           /* if this item is already in the list
                             * of initial references to process
                             */
                    return; /* we have removed it from the list and it will not be
                             * processed
                             */
                }

                std::size_t addingSize = adding.size();
                adding.remove(item);
                if (addingSize != adding.size())
                {           /* if the item is in the process of
                             * being added
                             */
                    return; /*
                             * in case the item is untracked while in the process of
                             * adding
                             */
                }
                auto trackedItemIter = tracked.find(item);
                // nothing to do, no item is being tracked.
                if (trackedItemIter == tracked.end())
                {
                    return;
                }
                /*
                 * must remove from tracker before
                 * calling customizer callback
                 */
                object = trackedItemIter->second;
                tracked.erase(item);
                Modified(); /* increment modification count */
            }
            /* Call customizer outside of synchronized region */
            CustomizerRemoved(item, related, object);
            /*
             * If the customizer throws an unchecked exception, it is safe to let it
             * propagate
             */
        }

        template <class S, class T, class R>
        std::size_t
        BundleAbstractTracked<S, T, R>::Size_unlocked() const
        {
            return tracked.size();
        }

        template <class S, class T, class R>
        bool
        BundleAbstractTracked<S, T, R>::IsEmpty_unlocked() const
        {
            return tracked.empty();
        }

        template <class S, class T, class R>
        std::optional<T>
        BundleAbstractTracked<S, T, R>::GetCustomizedObject_unlocked(S item) const
        {
            typename TrackingMap::const_iterator i = tracked.find(item);
            if (i != tracked.end())
            {
                return std::optional<T>(i->second);
            }

            return std::nullopt;
        }

        template <class S, class T, class R>
        void
        BundleAbstractTracked<S, T, R>::GetTracked_unlocked(std::vector<S>& items) const
        {
            for (auto& i : tracked)
            {
                items.push_back(i.first);
            }
        }

        template <class S, class T, class R>
        void
        BundleAbstractTracked<S, T, R>::Modified()
        {
            // atomic
            ++trackingCount;
        }

        template <class S, class T, class R>
        int
        BundleAbstractTracked<S, T, R>::GetTrackingCount() const
        {
            // atomic
            return trackingCount;
        }

        template <class S, class T, class R>
        void
        BundleAbstractTracked<S, T, R>::CopyEntries_unlocked(TrackingMap& map) const
        {
            map.insert(tracked.begin(), tracked.end());
        }

        template <class S, class T, class R>
        bool
        BundleAbstractTracked<S, T, R>::CustomizerAddingFinal(S item, std::optional<T> const& custom)
        {
            auto l = this->Lock();
            US_UNUSED(l);
            std::size_t addingSize = adding.size();
            adding.remove(item);
            if (addingSize != adding.size() && !closed)
            {
                /*
                 * if the item was not untracked during the customizer
                 * callback
                 */
                if (custom)
                {
                    tracked[item] = custom.value();
                    Modified();        /* increment modification count */
                    this->NotifyAll(); /* notify any waiters */
                }
                return false;
            }
            else
            {
                return true;
            }
        }

        template <class S, class T, class R>
        void
        BundleAbstractTracked<S, T, R>::TrackAdding(S item, R related)
        {
            std::optional<T> object;
            bool becameUntracked = false;
            /* Call customizer outside of synchronized region */
            try
            {
                object = CustomizerAdding(item, related);
                becameUntracked = this->CustomizerAddingFinal(item, object);
            }
            catch (...)
            {
                /*
                 * If the customizer throws an exception, it will
                 * propagate after the cleanup code.
                 */
                this->CustomizerAddingFinal(item, object);
                throw;
            }

            /*
             * The item became untracked during the customizer callback.
             */
            if (becameUntracked && object)
            {
                /* Call customizer outside of synchronized region */
                CustomizerRemoved(item, related, object.value());
                /*
                 * If the customizer throws an unchecked exception, it is safe to
                 * let it propagate
                 */
            }
        }

    } // namespace detail

} // namespace cppmicroservices
