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

#include "LDAPExpr.h"

#include "cppmicroservices/Any.h"
#include "cppmicroservices/Constants.h"

#include "Properties.h"
#include "Utils.h"

#include "PropsCheck.h"

#include <algorithm>
#include <bitset>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace cppmicroservices
{
    namespace
    {
#ifdef SUPPORT_NESTED_LOOKUP
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
        std::vector<std::string>
        string_split(std::string const& input, std::string_view delimiter_list)
        {
            std::vector<std::string> result;

            // Initialize a set of boolean flags indexed by ascii character value, one bit per
            // character. If the bit is 1, that character is a delimiter.
            std::bitset<255> delim;
            std::for_each(std::begin(delimiter_list), std::end(delimiter_list), [&delim](char c) { delim[c] = true; });

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
#endif

        /**
         * @brief Find value for attrName in map
         *
         * @tparam MapT the underlying storage class for the AnyMap.
         *
         * @param map an AnyMap with data to look up
         * @param attrName a string naming the attribute to find
         * @param get_value_from_map a function used to lookup values in map. Required because this
         *        algorithm does not know the underlying storage of the AnyMap
         * @param end_iter a function which returns the end iterator for the map. Required because
         *        this algorithm does not know the underlying storage of the AnyMap
         *
         * @return a std::optional const_iterator pointing to the found value. If the correct value
         *         is not found, and empty std::optional is returned.
         */
        template <typename MapT>
        std::optional<typename MapT::const_iterator>
        find_attr_value_in_map(
            AnyMap const* pPtr,
            std::string const& attrName,
            std::function<typename MapT::const_iterator(AnyMap const* p, std::string const& name)> get_value_from_map,
            std::function<typename MapT::const_iterator(AnyMap const* p)> end_iter)
        {
            // short ciruit check. See if the full attrName is defined at the top level and return
            // quickly if it is. We match this first to preserve existing behavior and only proceed
            // to "walk down" the JSON tree if we don't find the value at the top level.
            if (auto lookup = get_value_from_map(pPtr, attrName); lookup != end_iter(pPtr))
            {
                return lookup;
            }
#ifndef SUPPORT_NESTED_LOOKUP
            return std::nullopt;
#else
            // If not found at the full attrname, decompose the path and do a full check.
            // First, split the m_attrName into a vector at the . separator and reverse it.
            auto scope = string_split(attrName, ".");
            std::reverse(std::begin(scope), std::end(scope));

            // Now, the scope vector contains all the segments in the key in reverse
            // order. We are trying to "walk down" the AnyMap to a value.
            //
            // If m_attrName = "a.b.c.d",
            // 1. the scope vector is ["d","c","b","a"]
            // 2. pop "a" off the vector which now contains ["d","c","b"]
            // 3. check to see if "a" is a key in pPtr
            //    a. if it is a key, AND it's an any_map value, update pPtr to point to that
            //       nested any_map, and continue to walk down that any_map for the rest of
            //       the scope
            //    b. if it is a key, and it is NOT an any_map value, stop the map
            //       walking. If we've reached the bottom, then itr->second is what we want to
            //       compare. If not, m_attrName does not exist in the map
            // 4. If "a" is not a key in pPtr, repeat the steps in #3, but with "a.b" as the key.
            //
            // Basically, we need to check if there's a value at: "a.b.c","d", or
            // "a.b","c","d", or "a.b","c.d", or "a","b.c.d" or "a","b.c","d", or
            // "a","b","c.d", or "a","b","c.d", or "a","b","c","d"
            auto iter = end_iter(pPtr);
            std::string key {};
            std::string sep {};
            while (!scope.empty() && (iter == end_iter(pPtr)))
            {
                key += sep + scope.back();
                sep = ".";
                scope.pop_back();
                iter = get_value_from_map(pPtr, key);
                if (iter != end_iter(pPtr))
                {
                    // Attempt to cast the found value to an AnyMap.
                    pPtr = any_cast<AnyMap const>(&iter->second);

                    // If the found value is not an AnyMap, we can't "go down" the json tree any
                    // further, so break out of the loop
                    if (!pPtr)
                    {
                        break;
                    }

                    // If we get here, we've "walked down" another level in the json tree.
                    if (!scope.empty())
                    {
                        // If the scope is not yet empty, reset the key, sep, and iter variables to
                        // continue the walk down the json tree to the next level.
                        key = "";
                        sep = "";
                        iter = end_iter(pPtr);
                    }
                }
            }
            if (scope.empty() && !pPtr)
            {
                // If we get to this point, we have found the right value only if the entire attr
                // path has been processed, indicated by an empty scope vector AND a null pointer to
                // the map.
                return iter;
            }
            else
            {
                // return an empty object... we did not find a named attrName in the map.
                return std::nullopt;
            }
#endif
        }
    } // namespace

    namespace LDAPExprConstants
    {
        static LDAPExpr::Byte
        WILDCARD()
        {
            static LDAPExpr::Byte b = std::numeric_limits<LDAPExpr::Byte>::max();
            return b;
        }

        static std::string const&
        WILDCARD_STRING()
        {
            static std::string s(1, WILDCARD());
            return s;
        }

        static std::string const&
        NULLQ()
        {
            static std::string s = "Null query";
            return s;
        }

        static std::string const&
        GARBAGE()
        {
            static std::string s = "Trailing garbage";
            return s;
        }

        static std::string const&
        EOS()
        {
            static std::string s = "Unexpected end of query";
            return s;
        }

        static std::string const&
        MALFORMED()
        {
            static std::string s = "Malformed query";
            return s;
        }

        static std::string const&
        OPERATOR()
        {
            static std::string s = "Undefined operator";
            return s;
        }
    } // namespace LDAPExprConstants

    bool
    stricomp(std::string::value_type const& v1, std::string::value_type const& v2)
    {
        return ::tolower(v1) == ::tolower(v2);
    }

    //! Contains the current parser position and parsing utility methods.
    class LDAPExpr::ParseState
    {

      private:
        std::size_t m_pos;
        std::string m_str;

      public:
        ParseState(std::string const& str);

        //! Move m_pos to remove the prefix \a pre
        bool prefix(std::string const& pre);

        /** Peek a char at m_pos
        \note If index out of bounds, throw exception
        */
        LDAPExpr::Byte peek();

        //! Increment m_pos by n
        void skip(int n);

        //! return string from m_pos until the end
        std::string rest() const;

        //! Move m_pos until there's no spaces
        void skipWhite();

        //! Get string until special chars. Move m_pos
        std::string getAttributeName();

        //! Get string and convert * to WILDCARD
        std::string getAttributeValue();

        //! Throw InvalidSyntaxException exception
        void error(std::string const& m) const;
    };

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

        LDAPExprData(LDAPExprData const& other)

            = default;

        int m_operator;
        std::vector<LDAPExpr> m_args;
        std::string m_attrName;
        std::string m_attrValue;
    };

    LDAPExpr::LDAPExpr() : d() {}

    LDAPExpr::LDAPExpr(std::string const& filter) : d()
    {
        ParseState ps(filter);
        try
        {
            LDAPExpr expr = ParseExpr(ps);

            if (!Trim(ps.rest()).empty())
            {
                ps.error(StringCatFast(LDAPExprConstants::GARBAGE(), " '", ps.rest(), "'"));
            }

            d = expr.d;
        }
        catch (std::out_of_range const&)
        {
            ps.error(LDAPExprConstants::EOS());
        }
    }

    LDAPExpr::LDAPExpr(int op, std::vector<LDAPExpr> const& args) : d(std::make_shared<LDAPExprData>(op, args)) {}

    LDAPExpr::LDAPExpr(int op, std::string const& attrName, std::string const& attrValue)
        : d(std::make_shared<LDAPExprData>(op, attrName, attrValue))
    {
    }

    LDAPExpr::LDAPExpr(LDAPExpr const&) = default;

    LDAPExpr& LDAPExpr::operator=(LDAPExpr const&) = default;

    LDAPExpr::~LDAPExpr() = default;

    std::string
    LDAPExpr::Trim(std::string str)
    {
        str.erase(0, str.find_first_not_of(' '));
        auto const last_not_space = str.find_last_not_of(' ');
        if (last_not_space != std::string::npos)
        {
            str.erase(last_not_space + 1);
        }
        return str;
    }

    bool
    LDAPExpr::GetMatchedObjectClasses(ObjectClassSet& objClasses) const
    {
        if (d->m_operator == EQ)
        {
            if (d->m_attrName.length() == Constants::OBJECTCLASS.length()
                && std::equal(d->m_attrName.begin(), d->m_attrName.end(), Constants::OBJECTCLASS.begin(), stricomp)
                && d->m_attrValue.find(LDAPExprConstants::WILDCARD()) == std::string::npos)
            {
                objClasses.insert(d->m_attrValue);
                return true;
            }
            return false;
        }
        else if (d->m_operator == AND)
        {
            bool result = false;
            for (auto const& m_arg : d->m_args)
            {
                LDAPExpr::ObjectClassSet r;
                if (m_arg.GetMatchedObjectClasses(r))
                {
                    result = true;
                    if (objClasses.empty())
                    {
                        objClasses = r;
                    }
                    else
                    {
                        // if AND op and classes in several operands,
                        // then only the intersection is possible.
                        auto it1 = objClasses.begin();
                        auto it2 = r.begin();
                        while ((it1 != objClasses.end()) && (it2 != r.end()))
                        {
                            if (*it1 < *it2)
                            {
                                objClasses.erase(it1++);
                            }
                            else if (*it2 < *it1)
                            {
                                ++it2;
                            }
                            else
                            { // *it1 == *it2
                                ++it1;
                                ++it2;
                            }
                        }
                        // Anything left in set_1 from here on did not appear in set_2,
                        // so we remove it.
                        objClasses.erase(it1, objClasses.end());
                    }
                }
            }
            return result;
        }
        else if (d->m_operator == OR)
        {
            for (auto const& m_arg : d->m_args)
            {
                LDAPExpr::ObjectClassSet r;
                if (m_arg.GetMatchedObjectClasses(r))
                {
                    std::copy(r.begin(), r.end(), std::inserter(objClasses, objClasses.begin()));
                }
                else
                {
                    objClasses.clear();
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    std::string
    LDAPExpr::ToLower(std::string const& str)
    {
        std::string lowerStr(str);
        std::transform(str.begin(), str.end(), lowerStr.begin(), ::tolower);
        return lowerStr;
    }

    bool
    LDAPExpr::IsSimple(StringList const& keywords, LocalCache& cache, bool matchCase) const
    {
        if (cache.empty())
        {
            cache.resize(keywords.size());
        }

        if (d->m_operator == EQ)
        {
            StringList::const_iterator index;
            if ((index
                 = std::find(keywords.begin(), keywords.end(), matchCase ? d->m_attrName : ToLower(d->m_attrName)))
                    != keywords.end()
                && d->m_attrValue.find_first_of(LDAPExprConstants::WILDCARD()) == std::string::npos)
            {
                cache[index - keywords.begin()] = StringList(1, d->m_attrValue);
                return true;
            }
        }
        else if (d->m_operator == OR)
        {
            for (auto const& m_arg : d->m_args)
            {
                if (!m_arg.IsSimple(keywords, cache, matchCase))
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    bool
    LDAPExpr::IsNull() const
    {
        return !d;
    }

    bool
    LDAPExpr::Evaluate(PropertiesHandle const& p, bool matchCase) const
    {
        return Evaluate(p->GetPropsAnyMap(), matchCase);
    }

    bool
    LDAPExpr::Evaluate(AnyMap const& p, bool matchCase) const
    {
        // Use a ptr instead of a reference so we can "walk down" the json tree by reassigning it to
        // a nested level. You can't reassign references to point to a different object.
        AnyMap const* pPtr = &p;
        if ((d->m_operator & SIMPLE) != 0)
        {
            if (pPtr->GetType() == AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS)
            {
                auto value_iter = find_attr_value_in_map<any_map::unordered_any_cimap>(
                    pPtr,
                    d->m_attrName,
                    [](AnyMap const* p, std::string const& key) { return p->findUOCI_TypeChecked(key); },
                    [](AnyMap const* p) { return p->endUOCI_TypeChecked(); });

                if (!matchCase && value_iter)
                {
                    return Compare(value_iter.value()->second, d->m_operator, d->m_attrValue);
                }
                else if (matchCase && value_iter && value_iter.value()->first == d->m_attrName)
                {
                    return Compare(value_iter.value()->second, d->m_operator, d->m_attrValue);
                }
                else
                {
                    return false;
                }
            }
            else if (pPtr->GetType() == AnyMap::UNORDERED_MAP)
            {
                auto value_iter = find_attr_value_in_map<any_map::unordered_any_map>(
                    pPtr,
                    d->m_attrName,
                    [matchCase](AnyMap const* p, std::string const& key)
                    {
                        auto value_iter = p->findUO_TypeChecked(key);
                        if (!matchCase && value_iter == p->endUO_TypeChecked())
                        {
                            for (auto value_iter = p->beginUO_TypeChecked(); value_iter != p->endUO_TypeChecked();
                                 ++value_iter)
                            {
                                if (std::string lower = LDAPExpr::ToLower(key);
                                    LDAPExpr::ToLower(value_iter->first) == lower)
                                {
                                    return value_iter;
                                }
                            }
                            return p->endUO_TypeChecked();
                        }

                        return value_iter;
                    },
                    [](AnyMap const* p) { return p->endUO_TypeChecked(); });

                if (value_iter)
                {
                    return Compare(value_iter.value()->second, d->m_operator, d->m_attrValue);
                }

                return false;
            }
            else if (pPtr->GetType() == AnyMap::ORDERED_MAP)
            {
                auto value_iter = find_attr_value_in_map<any_map::ordered_any_map>(
                    pPtr,
                    d->m_attrName,
                    [matchCase](AnyMap const* p, std::string const& key)
                    {
                        auto value_iter = p->findOM_TypeChecked(key);
                        if (!matchCase && value_iter == p->endOM_TypeChecked())
                        {
                            for (auto value_iter = p->beginOM_TypeChecked(); value_iter != p->endOM_TypeChecked();
                                 ++value_iter)
                            {
                                if (std::string lower = LDAPExpr::ToLower(key);
                                    LDAPExpr::ToLower(value_iter->first) == lower)
                                {
                                    return value_iter;
                                }
                            }

                            return p->endOM_TypeChecked();
                        }

                        return value_iter;
                    },
                    [](AnyMap const* p) { return p->endOM_TypeChecked(); });

                if (value_iter)
                {
                    return Compare(value_iter.value()->second, d->m_operator, d->m_attrValue);
                }

                return false;
            }
            else
            {
                return false;
            }
        }
        else
        { // (d->m_operator & COMPLEX) != 0
            switch (d->m_operator)
            {
                case AND:
                    for (auto const& m_arg : d->m_args)
                    {
                        if (!m_arg.Evaluate(p, matchCase))
                        {
                            return false;
                        }
                    }
                    return true;
                case OR:
                    for (auto const& m_arg : d->m_args)
                    {
                        if (m_arg.Evaluate(p, matchCase))
                        {
                            return true;
                        }
                    }
                    return false;
                case NOT:
                    return !d->m_args[0].Evaluate(p, matchCase);
                default:
                    return false; // Cannot happen
            }
        }
    }

    bool
    LDAPExpr::Compare(Any const& obj, int op, std::string const& s) const
    {
        if (obj.Empty())
        {
            return false;
        }
        if (op == EQ && s == LDAPExprConstants::WILDCARD_STRING())
        {
            return true;
        }

        try
        {
            std::type_info const& objType = obj.Type();
            if (objType == typeid(std::string))
            {
                return CompareString(ref_any_cast<std::string>(obj), op, s);
            }
            else if (objType == typeid(char const*))
            {
                return CompareString(std::string(ref_any_cast<char const*>(obj)), op, s);
            }
            else if (objType == typeid(std::vector<std::string>))
            {
                auto const& list = ref_any_cast<std::vector<std::string>>(obj);
                for (std::size_t it = 0; it != list.size(); it++)
                {
                    if (CompareString(list[it], op, s))
                    {
                        return true;
                    }
                }
            }
            else if (objType == typeid(std::list<std::string>))
            {
                auto const& list = ref_any_cast<std::list<std::string>>(obj);
                for (auto const& it : list)
                {
                    if (CompareString(it, op, s))
                    {
                        return true;
                    }
                }
            }
            else if (objType == typeid(char))
            {
                return CompareString(std::string(1, ref_any_cast<char>(obj)), op, s);
            }
            else if (objType == typeid(bool))
            {
                if (op == LE || op == GE)
                {
                    return false;
                }

                std::string boolVal = any_cast<bool>(obj) ? "true" : "false";
                return std::equal(s.begin(), s.end(), boolVal.begin(), stricomp);
            }
            else if (objType == typeid(short))
            {
                return CompareIntegralType<short>(obj, op, s);
            }
            else if (objType == typeid(int))
            {
                return CompareIntegralType<int>(obj, op, s);
            }
            else if (objType == typeid(long int))
            {
                return CompareIntegralType<long int>(obj, op, s);
            }
            else if (objType == typeid(long long int))
            {
                return CompareIntegralType<long long int>(obj, op, s);
            }
            else if (objType == typeid(unsigned char))
            {
                return CompareIntegralType<unsigned char>(obj, op, s);
            }
            else if (objType == typeid(unsigned short))
            {
                return CompareIntegralType<unsigned short>(obj, op, s);
            }
            else if (objType == typeid(unsigned int))
            {
                return CompareIntegralType<unsigned int>(obj, op, s);
            }
            else if (objType == typeid(unsigned long int))
            {
                return CompareIntegralType<unsigned long int>(obj, op, s);
            }
            else if (objType == typeid(unsigned long long int))
            {
                return CompareIntegralType<unsigned long long int>(obj, op, s);
            }
            else if (objType == typeid(float))
            {
                errno = 0;
                char* endptr = nullptr;
                double sFloat = strtod(s.c_str(), &endptr);
                if ((errno == ERANGE && (sFloat == 0 || sFloat == HUGE_VAL || sFloat == -HUGE_VAL))
                    || (errno != 0 && sFloat == 0) || endptr == s.c_str())
                {
                    return false;
                }

                auto floatVal = static_cast<double>(any_cast<float>(obj));

                switch (op)
                {
                    case LE:
                        return floatVal <= sFloat;
                    case GE:
                        return floatVal >= sFloat;
                    default: /*APPROX and EQ*/
                        double diff = floatVal - sFloat;
                        return (diff < std::numeric_limits<float>::epsilon())
                               && (diff > -std::numeric_limits<float>::epsilon());
                }
            }
            else if (objType == typeid(double))
            {
                errno = 0;
                char* endptr = nullptr;
                double sDouble = strtod(s.c_str(), &endptr);
                if ((errno == ERANGE && (sDouble == 0 || sDouble == HUGE_VAL || sDouble == -HUGE_VAL))
                    || (errno != 0 && sDouble == 0) || endptr == s.c_str())
                {
                    return false;
                }

                auto doubleVal = any_cast<double>(obj);

                switch (op)
                {
                    case LE:
                        return doubleVal <= sDouble;
                    case GE:
                        return doubleVal >= sDouble;
                    default: /*APPROX and EQ*/
                        double diff = doubleVal - sDouble;
                        return (diff < std::numeric_limits<double>::epsilon())
                               && (diff > -std::numeric_limits<double>::epsilon());
                }
            }
            else if (objType == typeid(std::vector<Any>))
            {
                auto const& list = ref_any_cast<std::vector<Any>>(obj);
                for (std::size_t it = 0; it != list.size(); it++)
                {
                    if (Compare(list[it], op, s))
                    {
                        return true;
                    }
                }
            }
        }
        catch (...)
        {
            // This might happen if a std::string-to-datatype conversion fails
            // Just consider it a false match and ignore the exception
        }
        return false;
    }

    template <typename T>
    bool
    LDAPExpr::CompareIntegralType(Any const& obj, int const op, std::string const& s) const
    {
        errno = 0;
        char* endptr = nullptr;
        long longInt = strtol(s.c_str(), &endptr, 10);
        if ((errno == ERANGE
             && (longInt == std::numeric_limits<long>::max() || longInt == std::numeric_limits<long>::min()))
            || (errno != 0 && longInt == 0) || endptr == s.c_str())
        {
            return false;
        }

        auto sInt = static_cast<T>(longInt);
        auto intVal = any_cast<T>(obj);

        switch (op)
        {
            case LE:
                return intVal <= sInt;
            case GE:
                return intVal >= sInt;
            default: /*APPROX and EQ*/
                return intVal == sInt;
        }
    }

    bool
    LDAPExpr::CompareString(const std::string_view s1, int op, const std::string_view s2)
    {
        switch (op)
        {
            case LE:
                return s1.compare(s2) <= 0;
            case GE:
                return s1.compare(s2) >= 0;
            case EQ:
                return PatSubstr(s1, s2);
            case APPROX:
                return FixupString(s2) == FixupString(s1);
            default:
                return false;
        }
    }

    std::string
    LDAPExpr::FixupString(const std::string_view s)
    {
        std::string sb;
        sb.reserve(s.size());
        std::size_t len = s.length();
        for (std::size_t i = 0; i < len; i++)
        {
            char c = s.at(i);
            if (!std::isspace(c))
            {
                if (std::isupper(c))
                {
                    c = std::tolower(c);
                }
                sb.append(1, c);
            }
        }
        return sb;
    }

    bool
    LDAPExpr::PatSubstr(const std::string_view s, int si, const std::string_view pat, int pi)
    {
        if (pat.size() - pi == 0)
        {
            return s.size() - si == 0;
        }
        if (pat[pi] == LDAPExprConstants::WILDCARD())
        {
            pi++;
            for (;;)
            {
                if (PatSubstr(s, si, pat, pi))
                {
                    return true;
                }
                if (s.size() - si == 0)
                {
                    return false;
                }
                si++;
            }
        }
        else
        {
            if (s.size() - si == 0)
            {
                return false;
            }
            if (s[si] != pat[pi])
            {
                return false;
            }
            return PatSubstr(s, ++si, pat, ++pi);
        }
    }

    bool
    LDAPExpr::PatSubstr(const std::string_view s, const std::string_view pat)
    {
        return PatSubstr(s, 0, pat, 0);
    }

    LDAPExpr
    LDAPExpr::ParseExpr(ParseState& ps)
    {
        ps.skipWhite();
        if (!ps.prefix("("))
        {
            ps.error(LDAPExprConstants::MALFORMED());
        }

        int op;
        ps.skipWhite();
        Byte c = ps.peek();
        if (c == '&')
        {
            op = AND;
        }
        else if (c == '|')
        {
            op = OR;
        }
        else if (c == '!')
        {
            op = NOT;
        }
        else
        {
            return ParseSimple(ps);
        }
        ps.skip(1); // Ignore the d->m_operator

        std::vector<LDAPExpr> v;
        do
        {
            v.push_back(ParseExpr(ps));
            ps.skipWhite();
        } while (ps.peek() == '(');

        std::size_t n = v.size();
        if (!ps.prefix(")") || n == 0 || (op == NOT && n > 1))
        {
            ps.error(LDAPExprConstants::MALFORMED());
        }

        return LDAPExpr(op, v);
    }

    LDAPExpr
    LDAPExpr::ParseSimple(ParseState& ps)
    {
        std::string attrName = ps.getAttributeName();
        if (attrName.empty())
        {
            ps.error(LDAPExprConstants::MALFORMED());
        }
        int op = 0;
        if (ps.prefix("="))
        {
            op = EQ;
        }
        else if (ps.prefix("<="))
        {
            op = LE;
        }
        else if (ps.prefix(">="))
        {
            op = GE;
        }
        else if (ps.prefix("~="))
        {
            op = APPROX;
        }
        else
        {
            //      System.out.println("undef op='" + ps.peek() + "'");
            ps.error(LDAPExprConstants::OPERATOR()); // Does not return
        }
        std::string attrValue = ps.getAttributeValue();
        if (!ps.prefix(")"))
        {
            ps.error(LDAPExprConstants::MALFORMED());
        }
        return LDAPExpr(op, attrName, attrValue);
    }

    const std::string
    LDAPExpr::ToString() const
    {
        std::string res;
        res.append("(");
        if ((d->m_operator & SIMPLE) != 0)
        {
            res.append(d->m_attrName);
            switch (d->m_operator)
            {
                case EQ:
                    res.append("=");
                    break;
                case LE:
                    res.append("<=");
                    break;
                case GE:
                    res.append(">=");
                    break;
                case APPROX:
                    res.append("~=");
                    break;
            }

            for (char c : d->m_attrValue)
            {
                if (c == '(' || c == ')' || c == '*' || c == '\\')
                {
                    res.append(1, '\\');
                }
                else if (c == LDAPExprConstants::WILDCARD())
                {
                    c = '*';
                }
                res.append(1, c);
            }
        }
        else
        {
            switch (d->m_operator)
            {
                case AND:
                    res.append("&");
                    break;
                case OR:
                    res.append("|");
                    break;
                case NOT:
                    res.append("!");
                    break;
            }
            for (auto const& m_arg : d->m_args)
            {
                res.append(m_arg.ToString());
            }
        }
        res.append(")");
        return res;
    }

    LDAPExpr::ParseState::ParseState(std::string const& str) : m_pos(0), m_str()
    {
        if (str.empty())
        {
            error(LDAPExprConstants::NULLQ());
        }

        m_str = str;
    }

    bool
    LDAPExpr::ParseState::prefix(std::string const& pre)
    {
        std::string::iterator startIter = m_str.begin() + m_pos;
        if (!std::equal(pre.begin(), pre.end(), startIter))
        {
            return false;
        }
        m_pos += pre.size();
        return true;
    }

    char
    LDAPExpr::ParseState::peek()
    {
        if (m_pos >= m_str.size())
        {
            throw std::out_of_range("LDAPExpr");
        }
        return m_str.at(m_pos);
    }

    void
    LDAPExpr::ParseState::skip(int n)
    {
        m_pos += n;
    }

    std::string
    LDAPExpr::ParseState::rest() const
    {
        return m_str.substr(m_pos);
    }

    void
    LDAPExpr::ParseState::skipWhite()
    {
        while (std::isspace(peek()))
        {
            m_pos++;
        }
    }

    std::string
    LDAPExpr::ParseState::getAttributeName()
    {
        std::size_t start = m_pos;
        std::size_t n = 0;
        bool nIsSet = false;
        for (;; m_pos++)
        {
            Byte c = peek();
            if (c == '(' || c == ')' || c == '<' || c == '>' || c == '=' || c == '~')
            {
                break;
            }
            else if (!std::isspace(c))
            {
                n = m_pos - start + 1;
                nIsSet = true;
            }
        }
        if (!nIsSet)
        {
            return std::string();
        }
        return m_str.substr(start, n);
    }

    std::string
    LDAPExpr::ParseState::getAttributeValue()
    {
        int num_parens = 0;
        std::string sb;
        bool exit = false;
        while (!exit)
        {
            Byte c = peek();
            switch (c)
            {
                case '(':
                    ++num_parens;
                    sb.append(1, c);
                    break;
                case ')':
                    if (num_parens > 0)
                    {
                        --num_parens;
                        sb.append(1, c);
                    }
                    else
                    {
                        exit = true;
                    }
                    break;
                case '*':
                    sb.append(1, LDAPExprConstants::WILDCARD());
                    break;
                case '\\':
                    sb.append(1, m_str.at(++m_pos));
                    break;
                default:
                    sb.append(1, c);
                    break;
            }

            if (!exit)
            {
                m_pos++;
            }
        }
        return sb;
    }

    void
    LDAPExpr::ParseState::error(std::string const& m) const
    {
        std::string errorStr = StringCatFast(m, ": ", (m_str.empty() ? "" : m_str.substr(m_pos)));
        throw std::invalid_argument(errorStr);
    }
} // namespace cppmicroservices
