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

#include "absl/strings/str_cat.h"

#include "Properties.h"

#include "PropsCheck.h"

#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <utility>

namespace cppmicroservices
{

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
                ps.error(absl::StrCat(LDAPExprConstants::GARBAGE(), " '", ps.rest(), "'"));
            }

            d = expr.d;
        }
        catch (std::out_of_range const&)
        {
            ps.error(LDAPExprConstants::EOS());
        }
    }

    LDAPExpr::LDAPExpr(int op, std::vector<LDAPExpr> const& args) : d(new LDAPExprData(op, args)) {}

    LDAPExpr::LDAPExpr(int op, std::string const& attrName, std::string const& attrValue)
        : d(new LDAPExprData(op, attrName, attrValue))
    {
    }

    LDAPExpr::LDAPExpr(LDAPExpr const&) = default;

    LDAPExpr& LDAPExpr::operator=(LDAPExpr const&) = default;

    LDAPExpr::~LDAPExpr() = default;

    std::string
    LDAPExpr::Trim(std::string str)
    {
        str.erase(0, str.find_first_not_of(' '));
        str.erase(str.find_last_not_of(' ') + 1);
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
                    return false;
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
        if ((d->m_operator & SIMPLE) != 0)
        {
            auto v = p->Value_unlocked(d->m_attrName, matchCase);
            return (!v.second) ? false : Compare(v.first, d->m_operator, d->m_attrValue);
        }
        else
        { // (d->m_operator & COMPLEX) != 0
            switch (d->m_operator)
            {
                case AND:
                    for (auto const& m_arg : d->m_args)
                    {
                        if (!m_arg.Evaluate(p, matchCase))
                            return false;
                    }
                    return true;
                case OR:
                    for (auto const& m_arg : d->m_args)
                    {
                        if (m_arg.Evaluate(p, matchCase))
                            return true;
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
    LDAPExpr::Evaluate(AnyMap const& p, bool matchCase) const
    {
        if ((d->m_operator & SIMPLE) != 0)
        {
            if (p.GetType() == AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS)
            {
                auto itr = p.findUOCI_TypeChecked(d->m_attrName);
                if (!matchCase && itr != p.endUOCI_TypeChecked())
                {
                    return Compare(itr->second, d->m_operator, d->m_attrValue);
                }
                else if (matchCase && itr != p.endUOCI_TypeChecked() && itr->first == d->m_attrName)
                {
                    return Compare(itr->second, d->m_operator, d->m_attrValue);
                }
                else
                {
                    return false;
                }
            }
            else if (p.GetType() == AnyMap::UNORDERED_MAP)
            {
                auto itr = p.findUO_TypeChecked(d->m_attrName);
                if (itr != p.endUO_TypeChecked())
                {
                    return Compare(itr->second, d->m_operator, d->m_attrValue);
                }

                if (!matchCase)
                {
                    std::string const lower = LDAPExpr::ToLower(d->m_attrName);
                    for (auto itr = p.beginUO_TypeChecked(); itr != p.endUO_TypeChecked(); ++itr)
                    {
                        if (itr->first == lower)
                        {
                            return Compare(p.findUO_TypeChecked(lower)->second, d->m_operator, d->m_attrValue);
                        }
                    }
                    return false;
                }

                return false;
            }
            else if (p.GetType() == AnyMap::ORDERED_MAP)
            {
                auto itr = p.findOM_TypeChecked(d->m_attrName);
                if (itr != p.endOM_TypeChecked())
                {
                    return Compare(itr->second, d->m_operator, d->m_attrValue);
                }

                if (!matchCase)
                {
                    std::string const lower = LDAPExpr::ToLower(d->m_attrName);
                    for (auto itr = p.beginOM_TypeChecked(); itr != p.endOM_TypeChecked(); ++itr)
                    {
                        if (itr->first == lower)
                        {
                            return Compare(p.findOM_TypeChecked(lower)->second, d->m_operator, d->m_attrValue);
                        }
                    }
                    return false;
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
                            return false;
                    }
                    return true;
                case OR:
                    for (auto const& m_arg : d->m_args)
                    {
                        if (m_arg.Evaluate(p, matchCase))
                            return true;
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
            return false;
        if (op == EQ && s == LDAPExprConstants::WILDCARD_STRING())
            return true;

        try
        {
            std::type_info const& objType = obj.Type();
            if (objType == typeid(std::string))
            {
                return CompareString(ref_any_cast<std::string>(obj), op, s);
            }
            else if (objType == typeid(std::vector<std::string>))
            {
                auto const& list = ref_any_cast<std::vector<std::string>>(obj);
                for (std::size_t it = 0; it != list.size(); it++)
                {
                    if (CompareString(list[it], op, s))
                        return true;
                }
            }
            else if (objType == typeid(std::list<std::string>))
            {
                auto const& list = ref_any_cast<std::list<std::string>>(obj);
                for (auto const& it : list)
                {
                    if (CompareString(it, op, s))
                        return true;
                }
            }
            else if (objType == typeid(char))
            {
                return CompareString(std::string(1, ref_any_cast<char>(obj)), op, s);
            }
            else if (objType == typeid(bool))
            {
                if (op == LE || op == GE)
                    return false;

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
                        return true;
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
    LDAPExpr::CompareString(const absl::string_view s1, int op, const absl::string_view s2)
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
    LDAPExpr::FixupString(const absl::string_view s)
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
                    c = std::tolower(c);
                sb.append(1, c);
            }
        }
        return sb;
    }

    bool
    LDAPExpr::PatSubstr(const absl::string_view s, int si, const absl::string_view pat, int pi)
    {
        if (pat.size() - pi == 0)
            return s.size() - si == 0;
        if (pat[pi] == LDAPExprConstants::WILDCARD())
        {
            pi++;
            for (;;)
            {
                if (PatSubstr(s, si, pat, pi))
                    return true;
                if (s.size() - si == 0)
                    return false;
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
    LDAPExpr::PatSubstr(const absl::string_view s, const absl::string_view pat)
    {
        return PatSubstr(s, 0, pat, 0);
    }

    LDAPExpr
    LDAPExpr::ParseExpr(ParseState& ps)
    {
        ps.skipWhite();
        if (!ps.prefix("("))
            ps.error(LDAPExprConstants::MALFORMED());

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
            ps.error(LDAPExprConstants::MALFORMED());

        return LDAPExpr(op, v);
    }

    LDAPExpr
    LDAPExpr::ParseSimple(ParseState& ps)
    {
        std::string attrName = ps.getAttributeName();
        if (attrName.empty())
            ps.error(LDAPExprConstants::MALFORMED());
        int op = 0;
        if (ps.prefix("="))
            op = EQ;
        else if (ps.prefix("<="))
            op = LE;
        else if (ps.prefix(">="))
            op = GE;
        else if (ps.prefix("~="))
            op = APPROX;
        else
        {
            //      System.out.println("undef op='" + ps.peek() + "'");
            ps.error(LDAPExprConstants::OPERATOR()); // Does not return
        }
        std::string attrValue = ps.getAttributeValue();
        if (!ps.prefix(")"))
            ps.error(LDAPExprConstants::MALFORMED());
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
            return false;
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
        std::string errorStr = absl::StrCat(m, ": ", (m_str.empty() ? "" : m_str.substr(m_pos)));
        throw std::invalid_argument(errorStr);
    }
} // namespace cppmicroservices
