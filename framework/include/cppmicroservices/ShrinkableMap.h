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

#ifndef CPPMICROSERVICES_SHRINKABLEMAP_H
#define CPPMICROSERVICES_SHRINKABLEMAP_H

#include "cppmicroservices/GlobalConfig.h"

#include <map>

namespace cppmicroservices
{

    /**
     * \ingroup MicroServicesUtils
     *
     * A std::map style associative container allowing query and removal
     * operations only.
     */
    template <class Key, class T>
    class ShrinkableMap
    {
      private:
        static std::map<Key, T> emptyContainer;

      public:
        using container_type = std::map<Key, T>;
        using iterator = typename container_type::iterator;
        using const_iterator = typename container_type::const_iterator;
        using size_type = typename container_type::size_type;
        using key_type = typename container_type::key_type;
        using mapped_type = typename container_type::mapped_type;
        using value_type = typename container_type::value_type;
        using reference = typename container_type::reference;
        using const_reference = typename container_type::const_reference;

        ShrinkableMap() : container(emptyContainer) {}

        iterator
        begin()
        {
            return container.begin();
        }

        const_iterator
        begin() const
        {
            return container.begin();
        }

        iterator
        end()
        {
            return container.end();
        }

        const_iterator
        end() const
        {
            return container.end();
        }

        void
        erase(iterator pos)
        {
            container.erase(pos);
        }

        void
        erase(iterator first, iterator last)
        {
            container.erase(first, last);
        }

        size_type
        erase(Key const& key)
        {
            return container.erase(key);
        }

        bool
        empty() const
        {
            return container.empty();
        }

        void
        clear()
        {
            container.clear();
        }

        size_type
        size() const
        {
            return container.size();
        }

        size_type
        max_size() const
        {
            return container.max_size();
        }

        /**
         * \rststar
         * .. deprecated:: 3.1.0
         *
         *    This function exists only to maintain backwards compatibility
         *    and will be removed in the next major release.
         *    Use :any:`at(size_type pos) <cppmicroservices::ShrinkableMap::at>` instead.
         * \endrststar
         */
        US_DEPRECATED T&
        operator[](Key const& key)
        {
            return container[key];
        }

        T&
        at(Key const& key)
        {
            return container.at(key);
        }

        T const&
        at(Key const& key) const
        {
            return container.at(key);
        }

        size_type
        count(Key const& key) const
        {
            return container.count(key);
        }

        iterator
        find(Key const& key)
        {
            return container.find(key);
        }

        const_iterator
        find(Key const& key) const
        {
            return container.find(key);
        }

        std::pair<iterator, iterator>
        equal_range(Key const& key)
        {
            return container.equal_range(key);
        }

        std::pair<const_iterator, const_iterator>
        equal_range(Key const& key) const
        {
            return container.equal_range(key);
        }

        iterator
        lower_bound(Key const& key)
        {
            return container.lower_bound(key);
        }

        const_iterator
        lower_bound(Key const& key) const
        {
            return container.lower_bound(key);
        }

        iterator
        upper_bound(Key const& key)
        {
            return container.upper_bound(key);
        }

        const_iterator
        upper_bound(Key const& key) const
        {
            return container.upper_bound(key);
        }

      private:
        friend class ServiceHooks;

        ShrinkableMap(container_type& container) : container(container) {}

        container_type& container;
    };

    template <class Key, class T>
    std::map<Key, T> ShrinkableMap<Key, T>::emptyContainer;
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_SHRINKABLEMAP_H
