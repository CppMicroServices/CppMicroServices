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

#include "cppmicroservices/FrameworkConfig.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "cppmicroservices/AnyMap.h"

namespace cppmicroservices
{

    class Any;
    class LDAPExprData;
    class PropertiesHandle;

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

        //! Evaluate this LDAP filter.
        bool Evaluate(PropertiesHandle const& p, bool matchCase) const;

        // Evaluate this LDAP filter directly on an AnyMap rather than a PropertiesHandle.
        //
        // This function was added as an optimization since passing an AnyMap to the constructor of a
        // PropertiesHandle causes unnecessary copies to occurr.
        bool Evaluate(AnyMap const& p, bool matchCase) const;

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

        //! Shared pointer
        std::shared_ptr<LDAPExprData> d;
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_LDAPEXPR_H
