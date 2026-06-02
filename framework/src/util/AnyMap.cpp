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

#include "cppmicroservices/AnyMap.h"

#include <cassert>
#include <stdexcept>

namespace cppmicroservices
{

    namespace detail
    {

        std::size_t
        any_map_cihash::operator()(std::string const& key) const
        {
            std::string lcase = key;
            std::transform(lcase.begin(), lcase.end(), lcase.begin(), ::tolower);
            return std::hash<std::string> {}(lcase);
        }

        bool
        any_map_ciequal::operator()(std::string const& l, std::string const& r) const
        {
            return (
                l.size() == r.size()
                && std::equal(l.begin(), l.end(), r.begin(), [](char a, char b) { return tolower(a) == tolower(b); }));
        }

        Any const& AtCompoundKey(std::vector<Any> const& v, std::string_view const& key);

        Any const&
        AtCompoundKey(AnyMap const& m, std::string_view const& key)
        {
            auto pos = key.find(".");
            if (pos != std::string_view::npos)
            {
                auto head = key.substr(0, pos);
                auto tail = key.substr(pos + 1);

                auto& h = m.at(std::string(head));
                if (h.Type() == typeid(AnyMap))
                {
                    return AtCompoundKey(ref_any_cast<AnyMap>(h), tail);
                }
                else if (h.Type() == typeid(std::vector<Any>))
                {
                    return AtCompoundKey(ref_any_cast<std::vector<Any>>(h), tail);
                }
                throw std::invalid_argument("Unsupported Any type at '" + std::string(head) + "' for dotted get");
            }
            else
            {
                return m.at(std::string(key));
            }
        }

        Any const&
        AtCompoundKey(std::vector<Any> const& v, std::string_view const& key)
        {
            auto pos = key.find(".");
            if (pos != std::string_view::npos)
            {
                auto head = key.substr(0, pos);
                auto tail = key.substr(pos + 1);

                int const index = std::stoi(std::string(head));
                auto& h = v.at(index < 0 ? v.size() + index : index);

                if (h.Type() == typeid(AnyMap))
                {
                    return AtCompoundKey(ref_any_cast<AnyMap>(h), tail);
                }
                else if (h.Type() == typeid(std::vector<Any>))
                {
                    return AtCompoundKey(ref_any_cast<std::vector<Any>>(h), tail);
                }
                throw std::invalid_argument("Unsupported Any type at '" + std::string(head) + "' for dotted get");
            }
            else
            {
                int const index = std::stoi(std::string(key));
                return v.at(index < 0 ? v.size() + index : index);
            }
        }

        Any AtCompoundKey(std::vector<Any> const& v, std::string_view const& key, Any&& defaultVal);

        Any
        AtCompoundKey(AnyMap const& m, std::string_view const& key, Any&& defaultVal)
        {
            auto pos = key.find(".");
            if (pos != std::string_view::npos)
            {
                auto const head = key.substr(0, pos);
                auto const tail = key.substr(pos + 1);
                auto itr = m.find(std::string(head));
                if (itr != m.end())
                {
                    auto& h = itr->second;
                    if (h.Type() == typeid(AnyMap))
                    {
                        return AtCompoundKey(ref_any_cast<AnyMap>(h), tail, std::move(defaultVal));
                    }
                    else if (h.Type() == typeid(std::vector<Any>))
                    {
                        return AtCompoundKey(ref_any_cast<std::vector<Any>>(h), tail, std::move(defaultVal));
                    }
                }
            }
            else
            {
                auto itr = m.find(std::string(key));
                if (itr != m.end())
                {
                    return itr->second;
                }
            }
            return std::move(defaultVal);
        }

        Any
        AtCompoundKey(std::vector<Any> const& v, std::string_view const& key, Any&& defaultval)
        {
            auto pos = key.find(".");
            auto const head = key.substr(0, pos);
            auto const tail = (pos == std::string_view::npos) ? "" : key.substr(pos + 1);

            int index = 0;
            try
            {
                index = std::stoi(std::string(head));
            }
            catch (...)
            {
                return std::move(defaultval);
            }

            if (static_cast<size_t>(std::abs(index)) < v.size())
            {
                auto& h = v[(index < 0 ? v.size() + index : index)];
                if (tail.empty())
                {
                    return h;
                }
                else if (h.Type() == typeid(AnyMap))
                {
                    return AtCompoundKey(ref_any_cast<AnyMap>(h), tail, std::move(defaultval));
                }
                else if (h.Type() == typeid(std::vector<Any>))
                {
                    return AtCompoundKey(ref_any_cast<std::vector<Any>>(h), tail, std::move(defaultval));
                }
            }
            return std::move(defaultval);
        }
    } // namespace detail

    // ----------------------------------------------------------------
    // ------------------  AnyMap::const_iter  -------------------------

    AnyMap::const_iter::const_iter(AnyMap::iter const& it)
    {
        switch (it.it_.index())
        {
            case 0:
                it_ = std::monostate {};
                break;
            case 1:
                it_.emplace<1>(std::get<1>(it.it_));
                break;
            case 2:
                it_.emplace<2>(std::get<2>(it.it_));
                break;
            case 3:
                it_.emplace<3>(std::get<3>(it.it_));
                break;
        }
    }


    AnyMap::const_iter::reference
    AnyMap::const_iter::operator*() const
    {
        return std::visit(
            [](auto const& i) -> reference
            {
                using T = std::decay_t<decltype(i)>;
                if constexpr (std::is_same_v<T, std::monostate>)
                {
                    throw std::logic_error("cannot dereference an invalid iterator");
                }
                else
                {
                    return *i;
                }
            },
            it_);
    }

    AnyMap::const_iter::pointer
    AnyMap::const_iter::operator->() const
    {
        return &(**this);
    }

    AnyMap::const_iter::iterator&
    AnyMap::const_iter::operator++()
    {
        std::visit(
            [](auto& i)
            {
                using T = std::decay_t<decltype(i)>;
                if constexpr (std::is_same_v<T, std::monostate>)
                {
                    throw std::logic_error("cannot increment an invalid iterator");
                }
                else
                {
                    ++i;
                }
            },
            it_);
        return *this;
    }

    AnyMap::const_iter::iterator
    AnyMap::const_iter::operator++(int)
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    bool
    AnyMap::const_iter::operator==(iterator const& x) const
    {
        return it_ == x.it_;
    }

    bool
    AnyMap::const_iter::operator!=(iterator const& x) const
    {
        return !(*this == x);
    }

    // ----------------------------------------------------------------
    // ---------------------  AnyMap::iter  ----------------------------

    AnyMap::iter::reference
    AnyMap::iter::operator*() const
    {
        return std::visit(
            [](auto const& i) -> reference
            {
                using T = std::decay_t<decltype(i)>;
                if constexpr (std::is_same_v<T, std::monostate>)
                {
                    throw std::logic_error("cannot dereference an invalid iterator");
                }
                else
                {
                    return *i;
                }
            },
            it_);
    }

    AnyMap::iter::pointer
    AnyMap::iter::operator->() const
    {
        return &(**this);
    }

    AnyMap::iter::iterator&
    AnyMap::iter::operator++()
    {
        std::visit(
            [](auto& i)
            {
                using T = std::decay_t<decltype(i)>;
                if constexpr (std::is_same_v<T, std::monostate>)
                {
                    throw std::logic_error("cannot increment an invalid iterator");
                }
                else
                {
                    ++i;
                }
            },
            it_);
        return *this;
    }

    AnyMap::iter::iterator
    AnyMap::iter::operator++(int)
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    bool
    AnyMap::iter::operator==(iterator const& x) const
    {
        return it_ == x.it_;
    }

    bool
    AnyMap::iter::operator!=(iterator const& x) const
    {
        return !(*this == x);
    }

    // ----------------------------------------------------------
    // ------------------------  AnyMap  -------------------------

    AnyMap::AnyMap(std::initializer_list<value_type> l)
        : map_(unordered_any_cimap(l))
    {
    }

    AnyMap::AnyMap(map_type type, std::initializer_list<value_type> l)
    {
        switch (type)
        {
            case ORDERED_MAP:
                map_.emplace<ordered_any_map>(l);
                break;
            case UNORDERED_MAP:
                map_.emplace<unordered_any_map>(l);
                break;
            case UNORDERED_MAP_CASEINSENSITIVE_KEYS:
                map_.emplace<unordered_any_cimap>(l);
                break;
            default:
                throw std::logic_error("invalid map type");
        }
    }

    AnyMap::AnyMap(ordered_any_map const& m) : map_(m) {}
    AnyMap::AnyMap(ordered_any_map&& m) : map_(std::move(m)) {}
    AnyMap::AnyMap(unordered_any_map const& m) : map_(m) {}
    AnyMap::AnyMap(unordered_any_map&& m) : map_(std::move(m)) {}
    AnyMap::AnyMap(unordered_any_cimap const& m) : map_(m) {}
    AnyMap::AnyMap(unordered_any_cimap&& m) : map_(std::move(m)) {}

    AnyMap::map_type
    AnyMap::GetType() const
    {
        static constexpr map_type types[] = { ORDERED_MAP, UNORDERED_MAP, UNORDERED_MAP_CASEINSENSITIVE_KEYS };
        return types[map_.index()];
    }

    AnyMap::iterator
    AnyMap::begin()
    {
        switch (map_.index())
        {
            case 0:
                return iterator(iter::iter_variant(std::in_place_index<1>, std::get<0>(map_).begin()));
            case 1:
                return iterator(iter::iter_variant(std::in_place_index<2>, std::get<1>(map_).begin()));
            case 2:
                return iterator(iter::iter_variant(std::in_place_index<3>, std::get<2>(map_).begin()));
            default:
                throw std::logic_error("invalid map type");
        }
    }

    AnyMap::const_iter
    AnyMap::begin() const
    {
        switch (map_.index())
        {
            case 0:
                return const_iterator(const_iter::iter_variant(std::in_place_index<1>, std::get<0>(map_).begin()));
            case 1:
                return const_iterator(const_iter::iter_variant(std::in_place_index<2>, std::get<1>(map_).begin()));
            case 2:
                return const_iterator(const_iter::iter_variant(std::in_place_index<3>, std::get<2>(map_).begin()));
            default:
                throw std::logic_error("invalid map type");
        }
    }

    AnyMap::const_iterator
    AnyMap::cbegin() const
    {
        return begin();
    }

    AnyMap::iterator
    AnyMap::end()
    {
        switch (map_.index())
        {
            case 0:
                return iterator(iter::iter_variant(std::in_place_index<1>, std::get<0>(map_).end()));
            case 1:
                return iterator(iter::iter_variant(std::in_place_index<2>, std::get<1>(map_).end()));
            case 2:
                return iterator(iter::iter_variant(std::in_place_index<3>, std::get<2>(map_).end()));
            default:
                throw std::logic_error("invalid map type");
        }
    }

    AnyMap::const_iterator
    AnyMap::end() const
    {
        switch (map_.index())
        {
            case 0:
                return const_iterator(const_iter::iter_variant(std::in_place_index<1>, std::get<0>(map_).end()));
            case 1:
                return const_iterator(const_iter::iter_variant(std::in_place_index<2>, std::get<1>(map_).end()));
            case 2:
                return const_iterator(const_iter::iter_variant(std::in_place_index<3>, std::get<2>(map_).end()));
            default:
                throw std::logic_error("invalid map type");
        }
    }

    AnyMap::const_iterator
    AnyMap::cend() const
    {
        return end();
    }

    bool
    AnyMap::empty() const
    {
        return std::visit([](auto const& m) { return m.empty(); }, map_);
    }

    AnyMap::size_type
    AnyMap::size() const
    {
        return std::visit([](auto const& m) { return m.size(); }, map_);
    }

    AnyMap::size_type
    AnyMap::count(key_type const& key) const
    {
        return std::visit([&key](auto const& m) { return m.count(key); }, map_);
    }

    void
    AnyMap::clear()
    {
        std::visit([](auto& m) { m.clear(); }, map_);
    }

    AnyMap::mapped_type&
    AnyMap::at(key_type const& key)
    {
        return std::visit([&key](auto& m) -> mapped_type& { return m.at(key); }, map_);
    }

    AnyMap::mapped_type const&
    AnyMap::at(key_type const& key) const
    {
        return std::visit([&key](auto const& m) -> mapped_type const& { return m.at(key); }, map_);
    }

    AnyMap::mapped_type&
    AnyMap::operator[](key_type const& key)
    {
        return std::visit([&key](auto& m) -> mapped_type& { return m[key]; }, map_);
    }

    AnyMap::mapped_type&
    AnyMap::operator[](key_type&& key)
    {
        return std::visit([k = std::move(key)](auto& m) mutable -> mapped_type& { return m[std::move(k)]; }, map_);
    }

    std::pair<AnyMap::iterator, bool>
    AnyMap::insert(value_type const& value)
    {
        switch (map_.index())
        {
            case 0:
            {
                auto p = std::get<0>(map_).insert(value);
                return { iterator(iter::iter_variant(std::in_place_index<1>, std::move(p.first))), p.second };
            }
            case 1:
            {
                auto p = std::get<1>(map_).insert(value);
                return { iterator(iter::iter_variant(std::in_place_index<2>, std::move(p.first))), p.second };
            }
            case 2:
            {
                auto p = std::get<2>(map_).insert(value);
                return { iterator(iter::iter_variant(std::in_place_index<3>, std::move(p.first))), p.second };
            }
            default:
                throw std::logic_error("invalid map type");
        }
    }

    AnyMap::const_iterator
    AnyMap::find(key_type const& key) const
    {
        switch (map_.index())
        {
            case 0:
                return const_iterator(const_iter::iter_variant(std::in_place_index<1>, std::get<0>(map_).find(key)));
            case 1:
                return const_iterator(const_iter::iter_variant(std::in_place_index<2>, std::get<1>(map_).find(key)));
            case 2:
                return const_iterator(const_iter::iter_variant(std::in_place_index<3>, std::get<2>(map_).find(key)));
            default:
                throw std::logic_error("invalid map type");
        }
    }

    AnyMap::size_type
    AnyMap::erase(key_type const& key)
    {
        return std::visit([&key](auto& m) { return m.erase(key); }, map_);
    }

    bool
    AnyMap::operator==(AnyMap const& rhs) const
    {
        return map_ == rhs.map_;
    }

    AnyMap::mapped_type const&
    AnyMap::AtCompoundKey(key_type const& key) const
    {
        return detail::AtCompoundKey(*this, key);
    }

    AnyMap::mapped_type
    AnyMap::AtCompoundKey(key_type const& key, AnyMap::mapped_type defaultValue) const noexcept
    {
        return detail::AtCompoundKey(*this, key, std::move(defaultValue));
    }

    // ----------------------------------------------------------
    // -------------------  Serialization  ----------------------

    template <>
    std::ostream&
    any_value_to_string(std::ostream& os, AnyMap const& m)
    {
        os << "{";
        auto i1 = m.begin();
        auto const begin = i1;
        auto const end = m.end();
        for (; i1 != end; ++i1)
        {
            if (i1 == begin)
            {
                os << i1->first << " : " << i1->second.ToString();
            }
            else
            {
                os << ", " << i1->first << " : " << i1->second.ToString();
            }
        }
        os << "}";
        return os;
    }

    template <>
    std::ostream&
    any_value_to_json(std::ostream& os, AnyMap const& m, uint8_t const increment, int32_t const indent)
    {
        if (m.empty())
        {
            os << "{}";
            return os;
        }

        os << "{";
        auto i1 = m.begin();
        auto const begin = i1;
        auto const end = m.end();
        for (; i1 != end; ++i1)
        {
            if (i1 != begin)
            {
                os << ", ";
            }
            newline_and_indent(os, increment, indent);
            os << "\"" << i1->first << "\" : " << i1->second.ToJSON(increment, indent + increment);
        }
        newline_and_indent(os, increment, indent - increment);
        os << "}";
        return os;
    }

    template <>
    std::ostream&
    any_value_to_cpp(std::ostream& os, AnyMap const& m, uint8_t const increment, int32_t const indent)
    {
        auto const mapType = m.GetType();
        std::string typeStr;
        switch (mapType)
        {
            case AnyMap::ORDERED_MAP:
                typeStr = "ORDERED_MAP";
                break;
            case AnyMap::UNORDERED_MAP:
                typeStr = "UNORDERED_MAP";
                break;
            case AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS:
                typeStr = "UNORDERED_MAP_CASEINSENSITIVE_KEYS";
                break;
        }
        os << "AnyMap { " << typeStr << ", {";
        if (m.empty())
        {
            os << "}}";
            return os;
        }

        auto i1 = m.begin();
        auto const begin = i1;
        auto const end = m.end();
        for (; i1 != end; ++i1)
        {
            if (i1 != begin)
            {
                os << ", ";
            }
            newline_and_indent(os, increment, indent + increment);
            os << "{\"" << i1->first << "\" , " << i1->second.ToCPP(increment, indent + increment + increment) << "}";
        }
        newline_and_indent(os, increment, indent - increment);
        os << "}}";
        return os;
    }

} // namespace cppmicroservices
