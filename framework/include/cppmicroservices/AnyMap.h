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

#ifndef CPPMICROSERVICES_ANYMAP_H
#define CPPMICROSERVICES_ANYMAP_H

#include "cppmicroservices/Any.h"
#include <initializer_list>
#include <map>
#include <string>
#include <unordered_map>
#include <variant>

namespace cppmicroservices
{

    namespace detail
    {

        struct US_Framework_EXPORT any_map_cihash
        {
            std::size_t operator()(std::string const& key) const;
        };

        struct US_Framework_EXPORT any_map_ciequal
        {
            bool operator()(std::string const& l, std::string const& r) const;
        };

    } // namespace detail

    /**
     * \ingroup MicroServicesUtils
     *
     * A map data structure which wraps different STL map types.
     *
     * This class provides a STL associative container interface for different
     * underlying container types. Supported underlying types are:
     * - \c AnyMap::ordered_any_map (std::map)
     * - \c AnyMap::unordered_any_map (std::unordered_map)
     * - \c AnyMap::unordered_any_cimap (std::unordered_map with case-insensitive keys)
     *
     * Compound key access is supported via dotted notation (e.g., "three.b.1").
     */
    class US_Framework_EXPORT AnyMap
    {

      public:
        using key_type = std::string;
        using mapped_type = Any;
        using value_type = std::pair<key_type const, mapped_type>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = value_type const&;
        using pointer = value_type*;
        using const_pointer = value_type const*;
        using ordered_any_map = std::map<std::string, Any>;
        using unordered_any_map = std::unordered_map<std::string, Any>;
        using unordered_any_cimap
            = std::unordered_map<std::string, Any, detail::any_map_cihash, detail::any_map_ciequal>;
        using map_variant = std::variant<ordered_any_map, unordered_any_map, unordered_any_cimap>;

        enum map_type : uint8_t
        {
            ORDERED_MAP,
            UNORDERED_MAP,
            UNORDERED_MAP_CASEINSENSITIVE_KEYS
        };

        // ----- Iterator classes -----

        class iter;

        class US_Framework_EXPORT const_iter
        {
          public:
            using ociter = ordered_any_map::const_iterator;
            using uociter = unordered_any_map::const_iterator;
            using uocciiter = unordered_any_cimap::const_iterator;
            using iter_variant = std::variant<std::monostate, ociter, uociter, uocciiter>;

            using value_type = AnyMap::value_type;
            using reference = AnyMap::const_reference;
            using pointer = AnyMap::const_pointer;
            using iterator_category = std::forward_iterator_tag;
            using difference_type = AnyMap::difference_type;
            using iterator = const_iter;

            const_iter() = default;
            const_iter(const_iter const&) = default;
            const_iter& operator=(const_iter const&) = default;
            const_iter(iter const& it);

            explicit const_iter(iter_variant it) : it_(std::move(it)) {}

            reference operator*() const;
            pointer operator->() const;

            iterator& operator++();
            iterator operator++(int);

            bool operator==(iterator const& x) const;
            bool operator!=(iterator const& x) const;

          private:
            iter_variant it_;
        };

        class US_Framework_EXPORT iter
        {
          public:
            using oiter = ordered_any_map::iterator;
            using uoiter = unordered_any_map::iterator;
            using uociiter = unordered_any_cimap::iterator;
            using iter_variant = std::variant<std::monostate, oiter, uoiter, uociiter>;

            using value_type = AnyMap::value_type;
            using reference = AnyMap::reference;
            using pointer = AnyMap::pointer;
            using iterator_category = std::forward_iterator_tag;
            using difference_type = AnyMap::difference_type;
            using iterator = iter;

            iter() = default;
            iter(iter const&) = default;
            iter& operator=(iter const&) = default;

            explicit iter(iter_variant it) : it_(std::move(it)) {}

            reference operator*() const;
            pointer operator->() const;

            iterator& operator++();
            iterator operator++(int);

            bool operator==(iterator const& x) const;
            bool operator!=(iterator const& x) const;

          private:
            friend class const_iter;
            iter_variant it_;
        };

        using iterator = iter;
        using const_iterator = const_iter;

        // ----- Constructors -----

        AnyMap(std::initializer_list<value_type> l = {});
        AnyMap(map_type type, std::initializer_list<value_type> l = {});
        AnyMap(ordered_any_map const& m);
        AnyMap(ordered_any_map&& m);
        AnyMap(unordered_any_map const& m);
        AnyMap(unordered_any_map&& m);
        AnyMap(unordered_any_cimap const& m);
        AnyMap(unordered_any_cimap&& m);

        AnyMap(AnyMap const&) = default;
        AnyMap& operator=(AnyMap const&) = default;
        AnyMap(AnyMap&&) noexcept = default;
        AnyMap& operator=(AnyMap&&) noexcept = default;
        ~AnyMap() = default;

        // ----- Container interface -----

        iter begin();
        const_iter begin() const;
        const_iter cbegin() const;
        iter end();
        const_iter end() const;
        const_iter cend() const;

        bool empty() const;
        size_type size() const;
        size_type count(key_type const& key) const;
        void clear();

        mapped_type& at(key_type const& key);
        mapped_type const& at(key_type const& key) const;

        mapped_type& operator[](key_type const& key);
        mapped_type& operator[](key_type&& key);

        std::pair<iterator, bool> insert(value_type const& value);

        template <class... Args>
        std::pair<iterator, bool>
        emplace(Args&&... args)
        {
            switch (map_.index())
            {
                case 0:
                {
                    auto p = std::get<0>(map_).emplace(std::forward<Args>(args)...);
                    return { iterator(iter::iter_variant(std::in_place_index<1>, std::move(p.first))), p.second };
                }
                case 1:
                {
                    auto p = std::get<1>(map_).emplace(std::forward<Args>(args)...);
                    return { iterator(iter::iter_variant(std::in_place_index<2>, std::move(p.first))), p.second };
                }
                case 2:
                {
                    auto p = std::get<2>(map_).emplace(std::forward<Args>(args)...);
                    return { iterator(iter::iter_variant(std::in_place_index<3>, std::move(p.first))), p.second };
                }
                default:
                    throw std::logic_error("invalid map type");
            }
        }

        const_iterator find(key_type const& key) const;

        size_type erase(key_type const& key);

        bool operator==(AnyMap const& rhs) const;
        bool
        operator!=(AnyMap const& rhs) const
        {
            return !(operator==(rhs));
        }

        // ----- AnyMap-specific -----

        map_type GetType() const;

        mapped_type const& AtCompoundKey(key_type const& key) const;

        mapped_type AtCompoundKey(key_type const& key, mapped_type defaultValue) const noexcept;

        // ----- Public variant storage -----
        map_variant map_;
    };

    using any_map = AnyMap;

    template <>
    US_Framework_EXPORT std::ostream& any_value_to_string(std::ostream& os, AnyMap const& m);

    template <>
    US_Framework_EXPORT std::ostream& any_value_to_json(std::ostream& os,
                                                        AnyMap const& m,
                                                        uint8_t const increment,
                                                        int32_t const indent);

    template <>
    US_Framework_EXPORT std::ostream& any_value_to_cpp(std::ostream& os,
                                                       AnyMap const& m,
                                                       uint8_t const increment,
                                                       int32_t const indent);

} // namespace cppmicroservices

#endif // CPPMICROSERVICES_ANYMAP_H
