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

#ifndef CPPMICROSERVICES_LDAPEXPR_H
#define CPPMICROSERVICES_LDAPEXPR_H

#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/FrameworkConfig.h"

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace cppmicroservices
{
    class Any;
    class LDAPExprData;
    class PropertiesHandle;

    class LDAPExprData
    {
      public:
        LDAPExprData(int op, std::vector<LDAPExpr> args)
            : m_operator(op)
            , m_args(std::move(args))
            , m_attrName()
            , m_attrValue()
        {
        }

        LDAPExprData(int op, std::string attrName, std::string attrValue)
            : m_operator(op)
            , m_args()
            , m_attrName(std::move(attrName))
            , m_attrValue(std::move(attrValue))
        {
        }

        LDAPExprData(LDAPExprData const& other) = default;

        int m_operator;
        std::vector<LDAPExpr> m_args;
        std::string m_attrName;
        std::string m_attrValue;
    };

    /**
     * This class is not part of the public API.
     */
    class LDAPExpr
    {

      public:
        const static int AND = 0;
        const static int OR = 1;
        const static int NOT = 2;
        const static int EQ = 4;
        const static int LE = 8;
        const static int GE = 16;
        const static int APPROX = 32;
        const static int COMPLEX = AND | OR | NOT;
        const static int SIMPLE = EQ | LE | GE | APPROX;

        using Byte = char;
        using StringList = std::vector<std::string>;
        using LocalCache = std::vector<StringList>;
        using ObjectClassSet = std::unordered_set<std::string>;

        /** class that provides algorithm for a flat lookup in an AnyMap. See below. */
        template <typename MapT>
        class FlatLookup;

        /** class that provides algorithm for a nested lookup in an AnyMap. See below. */
        template <typename MapT>
        class NestedLookup;

        /**
         * Creates an invalid LDAPExpr object. Use with care.
         *
         * @see IsNull()
         */
        LDAPExpr();

        LDAPExpr(std::string const& filter);

        LDAPExpr(LDAPExpr const& other);

        LDAPExpr& operator=(LDAPExpr const& other);

        ~LDAPExpr();

        /**
         * Get object class set matched by this LDAP expression. This will not work
         * with wildcards and NOT expressions. If a set can not be determined return <code>false</code>.
         *
         * \param objClasses The set of matched classes will be added to objClasses.
         * \return If the set cannot be determined, <code>false</code> is returned, <code>true</code> otherwise.
         */
        bool GetMatchedObjectClasses(ObjectClassSet& objClasses) const;

        /**
         * Checks if this LDAP expression is "simple". The definition of
         * a simple filter is:
         * <ul>
         *  <li><code>(<it>name</it>=<it>value</it>)</code> is simple if
         *      <it>name</it> is a member of the provided <code>keywords</code>,
         *      and <it>value</it> does not contain a wildcard character;</li>
         *  <li><code>(| EXPR+ )</code> is simple if all <code>EXPR</code>
         *      expressions are simple;</li>
         *  <li>No other expressions are simple.</li>
         * </ul>
         * If the filter is found to be simple, the <code>cache</code> is
         * filled with mappings from the provided keywords to lists
         * of attribute values. The keyword-value-pairs are the ones that
         * satisfy this expression, for the given keywords.
         *
         * @param keywords The keywords to look for.
         * @param cache An array (indexed by the keyword indexes) of lists to
         * fill in with values saturating this expression.
         * @return <code>true</code> if this expression is simple,
         * <code>false</code> otherwise.
         */
        bool IsSimple(StringList const& keywords, LocalCache& cache, bool matchCase) const;

        /**
         * Returns <code>true</code> if this instance is invalid, i.e. it was
         * constructed using LDAPExpr().
         *
         * @return <code>true</code> if the expression is invalid,
         *         <code>false</code> otherwise.
         */
        bool IsNull() const;

        /**
         * Evaluate this LDAPExpr directly on an AnyMap.
         *
         * @tparam MapT a specific map implementation. One of any_map::unordered_any_cimap,
         *         any_map::unordered_any_map, any_map::ordered_any_map.
         * @tparam LookupT a class providing a function call overload that implements a specific
         *         algorithm for looking up a value in a give MapT. The two algorithms supported
         *         today are encapsulated below in the FlatLookup and NestedLookup classes.
         * @param  p the map in which to lookup the named value
         * @param  matchCase passed along to the lookup_value function to indicate whether or not
         *         the case of the key should be forced to match
         * @return a boolean indicating whether or not "p" matches against this LDAPExpr
         * This function was added as an optimization since passing an AnyMap to the constructor of a
         * PropertiesHandle causes unnecessary copies to occur.
         */
        template <template <typename MapT> class LookupT = FlatLookup>
        bool
        Evaluate(AnyMap const& p, bool matchCase) const
        {
            // Use a ptr instead of a reference so we can "walk down" the json tree by reassigning it to
            // a nested level. You can't reassign references to point to a different object.
            AnyMap const* pPtr = &p;
            if ((d->m_operator & SIMPLE) != 0)
            {
                if (pPtr->GetType() == AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS)
                {
                    LookupT<any_map::unordered_any_cimap> filter;
                    auto value_iter = filter(pPtr, d->m_attrName, matchCase);

                    if (!matchCase && value_iter)
                    {
                        return Compare(value_iter.value()->second, d->m_operator, d->m_attrValue);
                    }
                    else if (matchCase && value_iter && value_iter.value()->first == d->m_attrName)
                    {
                        return Compare(value_iter.value()->second, d->m_operator, d->m_attrValue);
                    }
                    return false;
                }
                else if (pPtr->GetType() == AnyMap::UNORDERED_MAP)
                {
                    LookupT<any_map::unordered_any_map> filter;
                    auto value_iter = filter(pPtr, d->m_attrName, matchCase);

                    if (value_iter)
                    {
                        return Compare(value_iter.value()->second, d->m_operator, d->m_attrValue);
                    }
                    return false;
                }
                else if (pPtr->GetType() == AnyMap::ORDERED_MAP)
                {
                    LookupT<any_map::ordered_any_map> filter;
                    auto value_iter = filter(pPtr, d->m_attrName, matchCase);

                    if (value_iter)
                    {
                        return Compare(value_iter.value()->second, d->m_operator, d->m_attrValue);
                    }
                    return false;
                }
                return false;
            }
            else
            { // (d->m_operator & COMPLEX) != 0
                switch (d->m_operator)
                {
                    case AND:
                        for (auto const& m_arg : d->m_args)
                        {
                            if (!m_arg.Evaluate<LookupT>(p, matchCase))
                                return false;
                        }
                        return true;
                    case OR:
                        for (auto const& m_arg : d->m_args)
                        {
                            if (m_arg.Evaluate<LookupT>(p, matchCase))
                                return true;
                        }
                        return false;
                    case NOT:
                        return !d->m_args[0].Evaluate<LookupT>(p, matchCase);
                    default:
                        return false; // Cannot happen
                }
            }
        }

        //!
        const std::string ToString() const;

      private:
        class ParseState;

        //!
        LDAPExpr(int op, std::vector<LDAPExpr> const& args);

        //!
        LDAPExpr(int op, std::string const& attrName, std::string const& attrValue);

        //!
        static LDAPExpr ParseExpr(ParseState& ps);

        //!
        static LDAPExpr ParseSimple(ParseState& ps);

        static std::string Trim(std::string str);

        static std::string ToLower(std::string const& str);

        //!
        bool Compare(Any const& obj, int op, std::string const& s) const;

        //!
        template <typename T>
        bool CompareIntegralType(Any const& obj, int const op, std::string const& s) const;

        //!
        static bool CompareString(const std::string_view s1, int op, const std::string_view s2);

        //!
        static std::string FixupString(const std::string_view s);

        //!
        static bool PatSubstr(const std::string_view s, const std::string_view pat);

        //!
        static bool PatSubstr(const std::string_view s, int si, const std::string_view pat, int pi);

        //! Shared pointer to object containing information about a parsed the LDAPExpr
        std::shared_ptr<LDAPExprData> d;

        /** Lookup a value in an AnyMap
         *
         * @tparam MapT a specific map implementation. One of any_map::unordered_any_cimap,
         *         any_map::unordered_any_map, any_map::ordered_any_map.
         * @param  p a pointer to an AnyMap to lookup the named Value
         * @param  key a string naming the value to find
         * @param  matchCase a boolean indicating whether or not the key must match with case
         *
         * @return a const iterator to the matching value or the map's end itertor
         */
        template <typename MapT>
        static auto
        lookup_value(AnyMap const* /* p */, std::string const& /* key */, bool /* matchCase */) ->
            typename MapT::const_iterator
        {
            /* Required implementation even though we specialize all supported MapT's */
            return nullptr;
        }

        /** Return the end_iter for the specified map
         *
         * @tparam MapT a specific map implementation. One of any_map::unordered_any_cimap,
         *         any_map::unordered_any_map, any_map::ordered_any_map.
         * @param  p a pointer to an AnyMap to lookup the named Value
         *
         * @return a the end iterator for the the given map.
         */
        template <typename MapT>
        static auto
        end_iter(AnyMap const* /* p */) -> typename MapT::const_iterator
        {
            /* Required implementation even though we specialize all supported MapT's */
            return nullptr;
        }

        /** Template specialization to perform the lookup in an unordered_any_cimap */
        template <>
        auto
        lookup_value<any_map::unordered_any_cimap>(AnyMap const* p, std::string const& key, bool)
            -> any_map::unordered_any_cimap::const_iterator
        {
            return p->findUOCI_TypeChecked(key);
        }

        /** Template specialization to get the end iterator for the unordered_any_cimap argument */
        template <>
        auto
        end_iter<any_map::unordered_any_cimap>(AnyMap const* p) -> any_map::unordered_any_cimap::const_iterator
        {
            return p->endUOCI_TypeChecked();
        }

        /** Template specialization to perform the lookup in an unordered_any_map */
        template <>
        auto
        lookup_value<any_map::unordered_any_map>(AnyMap const* p, std::string const& key, bool matchCase)
            -> any_map::unordered_any_map::const_iterator
        {
            auto value_iter = p->findUO_TypeChecked(key);
            if (!matchCase && value_iter == p->endUO_TypeChecked())
            {
                for (auto value_iter = p->beginUO_TypeChecked(); value_iter != p->endUO_TypeChecked(); ++value_iter)
                {
                    if (std::string lower = LDAPExpr::ToLower(key); LDAPExpr::ToLower(value_iter->first) == lower)
                    {
                        return value_iter;
                    }
                }
                return p->endUO_TypeChecked();
            }

            return value_iter;
        }

        /** Template specialization to get the end iterator for the unordered_any_map argument */
        template <>
        auto
        end_iter<any_map::unordered_any_map>(AnyMap const* p) -> any_map::unordered_any_map::const_iterator
        {
            return p->endUO_TypeChecked();
        }

        /** Template specialization to perform the lookup in an ordered_any_map */
        template <>
        auto
        lookup_value<any_map::ordered_any_map>(AnyMap const* p, std::string const& key, bool matchCase)
            -> any_map::ordered_any_map::const_iterator
        {
            auto value_iter = p->findOM_TypeChecked(key);
            if (!matchCase && value_iter == p->endOM_TypeChecked())
            {
                for (auto value_iter = p->beginOM_TypeChecked(); value_iter != p->endOM_TypeChecked(); ++value_iter)
                {
                    if (std::string lower = LDAPExpr::ToLower(key); LDAPExpr::ToLower(value_iter->first) == lower)
                    {
                        return value_iter;
                    }
                }

                return p->endOM_TypeChecked();
            }

            return value_iter;
        }

        /** Template specialization to get the end iterator for the ordered_any_map argument */
        template <>
        auto
        end_iter<any_map::ordered_any_map>(AnyMap const* p) -> any_map::ordered_any_map::const_iterator
        {
            return p->endOM_TypeChecked();
        }

      public:
        /**
         * Templated class to provide flat lookups in an anymap with a property name.
         *
         * @tparam MapT the underlying map type used for storage in the AnyMap, one of
         *         any_map::ordered_any_map
         *         any_map::unordered_any_map
         *         any_map::using unordered_any_cimap
         */
        template <typename MapT>
        class FlatLookup
        {
          public:
            /** function call operator implementing a flat lookup on attrName
             *
             * @param pPtr a pointer to an anymap
             * @param attrName the name of the property to lookup
             * @param matchCase a boolean indicating whether or not the attrName must match case.
             */
            std::optional<typename MapT::const_iterator>
            operator()(AnyMap const* pPtr, std::string const& attrName, bool matchCase) const
            {
                // short ciruit check. See if the full attrName is defined at the top level and return
                // quickly if it is. We match this first to preserve existing behavior and only proceed
                // to "walk down" the JSON tree if we don't find the value at the top level.
                if (auto lookup = lookup_value<MapT>(pPtr, attrName, matchCase); lookup != end_iter<MapT>(pPtr))
                {
                    return lookup;
                }
                return std::nullopt;
            }
        };

        /**
         * Templated class to provide nested lookups in an anymap on a dot separated property name.
         *
         * @tparam MapT the underlying map type used for storage in the AnyMap, one of
         *         any_map::ordered_any_map
         *         any_map::unordered_any_map
         *         any_map::using unordered_any_cimap
         */
        template <typename MapT>
        class NestedLookup
        {
          public:
            /** function call operator implementing nested value lookup
             *
             * @param pPtr a pointer to an anymap
             * @param attrName the name of a property to lookup. The name is a dot separated list of
             *        keys into submaps of pPtr
             * @param matchCase a boolean indicating whether or not attrName must match case
             * @return an optional iterator pointing to the found value or std::nullopt if the value
             *        named by attrName is not present in pPtr.
             */
            std::optional<typename MapT::const_iterator>
            operator()(AnyMap const* pPtr, std::string const& attrName, bool matchCase) const
            {
                // If not found at the full attrname, decompose the path and do a full check.
                // First, split the m_attrName into a vector at the . separator and reverse it.
                auto level_key = string_split(attrName, ".");
                std::reverse(std::begin(level_key), std::end(level_key));
                std::string key {};
                auto iter = end_iter<MapT>(pPtr);
                while (!level_key.empty())
                {
                    key = level_key.back();
                    iter = lookup_value<MapT>(pPtr, key, matchCase);
                    if (iter == end_iter<MapT>(pPtr))
                        break;

                    level_key.pop_back();

                    // Attempt to cast the found value to an AnyMap.
                    pPtr = any_cast<AnyMap const>(&iter->second);
                    if (nullptr == pPtr)
                        break;
                }
                if (level_key.empty() && !pPtr)
                {
                    // If we get to this point, we have found the right value only if the entire attr
                    // path has been processed, indicated by an empty scope vector AND a null pointer to
                    // the map.
                    return iter;
                }
                return std::nullopt;
            }

          private:
            /**
             * @brief split a delimited string into a vector of values
             *
             * String split from: https://stackoverflow.com/a/5506223/13030801
             * This is the fasted string split algorithm that we could find.
             *
             * @param input a delimited string to split
             * @param delimiter_list a string_view of characters representing delimiters
             *
             * @return a std::vector<std::string> of the delimited items from input
             */
            static std::vector<std::string>
            string_split(std::string const& input, std::string_view delimiter_list)
            {
                std::vector<std::string> result;

                // Initialize a set of boolean flags indexed by ascii character value, one bit per
                // character. If the bit is 1, that character is a delimiter.
                std::bitset<255> delim;
                std::for_each(std::begin(delimiter_list),
                              std::end(delimiter_list),
                              [&delim](char c) { delim[c] = true; });

                std::string::const_iterator beg;
                bool in_token = false;
                // Loop through the input string and check each character for a delimiter. If a
                // delimiter is found, add the string processed so far to the result container.
                for (auto it = std::begin(input); it != std::end(input); std::advance(it, 1))
                {
                    // If *it is a delimiter, the value marked by the delimiter is the string between
                    // "beg" and "it"... save it off into the result container.
                    if (delim[*it])
                    {
                        if (in_token)
                        {
                            // Only store a token if we're actually parsing a token.
                            result.push_back(std::string(beg, it));
                            in_token = false;
                        }
                    }
                    else if (!in_token)
                    {
                        // Found a non-delimiter character. If we're not currently processing a token,
                        // mark that we've found the beginning of a token so the next characters are
                        // part of the token.
                        beg = it;
                        in_token = true;
                    }
                }
                // deal with the boundary condition... the last token in input.
                if (in_token)
                {
                    result.push_back(std::string(beg, std::end(input)));
                }

                return result;
            }
        };
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_LDAPEXPR_H
