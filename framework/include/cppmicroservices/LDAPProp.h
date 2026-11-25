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

#ifndef CPPMICROSERVICES_LDAPPROP_H
#define CPPMICROSERVICES_LDAPPROP_H

#include "cppmicroservices/Any.h"
#include "cppmicroservices/FrameworkConfig.h"

#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4251)
#endif

namespace cppmicroservices
{

    /**
     * \ingroup MicroServicesUtils
     * \ingroup gr_ldap
     *
     * An LDAP filter expression, as created by the LDAPProp fluent API.
     *
     * LDAPPropExpr objects are usually not created directly. They are returned by
     * the LDAPProp operators and can be combined with the logical operators
     * <code>&&</code>, <code>||</code> and <code>!</code>.
     *
     * An LDAPPropExpr is implicitly convertible to <code>std::string</code>, which yields
     * the LDAP filter string. This allows passing it anywhere a filter string is expected,
     * such as the LDAPFilter constructor or BundleContext::GetServiceReferences:
     * \code
     * LDAPPropExpr expr = LDAPProp("name") == "Ben" && LDAPProp("count") >= 2;
     *
     * std::string filterString = expr; // "(&(name=Ben)(count>=2))"
     * LDAPFilter filter(expr);
     * \endcode
     *
     * \sa LDAPProp
     * \sa LDAPFilter
     */
    class US_Framework_EXPORT LDAPPropExpr
    {
      public:
        /**
         * Create a null (empty) expression.
         *
         * \sa IsNull
         */
        LDAPPropExpr();

        /**
         * Create an expression from an LDAP filter string.
         *
         * The string is stored as-is; it is not validated.
         *
         * @param expr An LDAP filter string.
         */
        explicit LDAPPropExpr(std::string expr);

        /**
         * Negates this expression in place.
         *
         * Wraps the expression as "(!expr)" and returns a reference to this object;
         * the expression itself is modified. A null expression is left unchanged.
         *
         * @return A reference to this LDAP expression object.
         */
        LDAPPropExpr& operator!();

        /**
         * Converts this expression to its LDAP filter string.
         *
         * The conversion is implicit, so an LDAPPropExpr can be used wherever a
         * <code>std::string</code> is expected:
         * \code
         * std::string filter = LDAPProp("key1") == "value1"; // "(key1=value1)"
         * \endcode
         *
         * @return The LDAP filter string, or an empty string if the expression is null.
         */
        operator std::string() const;

        /**
         * Checks whether this expression is null.
         *
         * @return <code>true</code> if the expression is empty (converts to an empty
         *         string), <code>false</code> otherwise.
         */
        bool IsNull() const;

        /**
         * Convenience operator for LDAP logical or '|'.
         *
         * Writing either
         * \code
         * LDAPPropExpr expr(LDAPProp("key1") == "value1");
         * expr = expr || LDAPProp("key2") == "value2";
         * \endcode
         * or
         * \code
         * LDAPPropExpr expr(LDAPProp("key1") == "value1");
         * expr |= LDAPProp("key2") == "value2";
         * \endcode
         * leads to the same string "(|(key1=value1) (key2=value2))".
         *
         * @param right A LDAP expression object.
         * @return A LDAP expression object.
         *
         * @{
         */
        LDAPPropExpr& operator|=(LDAPPropExpr const& right);
        /// @}

        /**
         * Convenience operator for LDAP logical and '&'.
         *
         * Writing either
         * \code
         * LDAPPropExpr expr(LDAPProp("key1") == "value1");
         * expr = expr && LDAPProp("key2") == "value2";
         * \endcode
         * or
         * \code
         * LDAPPropExpr expr(LDAPProp("key1") == "value1");
         * expr &= LDAPProp("key2") == "value2";
         * \endcode
         * leads to the same string "(&(key1=value1) (key2=value2))".
         *
         * @param right A LDAP expression object.
         * @return A LDAP expression object.
         *
         * @{
         */
        LDAPPropExpr& operator&=(LDAPPropExpr const& right);
        /// @}

      private:
        std::string m_ldapExpr;
    };

    /**
     * \ingroup MicroServicesUtils
     * \ingroup gr_ldap
     *
     * A fluent API for creating LDAP filter strings.
     *
     * Examples for creating LDAPFilter objects:
     * \code
     * // This creates the filter "(&(name=Ben)(!(count=1)))"
     * LDAPFilter filter(LDAPProp("name") == "Ben" && !(LDAPProp("count") == 1));
     *
     * // This creates the filter "(|(presence=*)(!(absence=*)))"
     * LDAPFilter filter(LDAPProp("presence") || !LDAPProp("absence"));
     *
     * // This creates the filter "(&(ge>=-3)(approx~=hi))"
     * LDAPFilter filter(LDAPProp("ge") >= -3 && LDAPProp("approx").Approx("hi"));
     * \endcode
     *
     * \sa LDAPFilter
     */
    class US_Framework_EXPORT LDAPProp
    {
      public:
        /**
         * Create a LDAPProp instance for the named LDAP property.
         *
         * @param property The name of the LDAP property.
         */
        LDAPProp(std::string property);

        /**
         * \addtogroup gr_ldap
         * LDAP equality '='
         *
         * @param s A type convertible to std::string.
         * @param b A type convertible to bool
         * @return A LDAP expression object.
         *
         * @{
         */
        LDAPPropExpr operator==(std::string const& s) const;
        LDAPPropExpr operator==(cppmicroservices::Any const& s) const;
        LDAPPropExpr operator==(bool b) const;
        template <class T>
        LDAPPropExpr
        operator==(T const& s) const
        {
            std::stringstream ss;
            ss << s;
            return LDAPPropExpr("(" + m_property + "=" + ss.str() + ")");
        }
        /// @}

        /**
         * States the presence of the LDAP property.
         *
         * Converting <code>LDAPProp("attr")</code> to an LDAPPropExpr yields "(attr=*)".
         *
         * @return A LDAP expression object.
         */
        operator LDAPPropExpr() const;

        /**
         * States the absence of the LDAP property.
         *
         * @return A LDAP expression object.
         */
        LDAPPropExpr operator!() const;

        /**
         * \addtogroup gr_ldap
         * Convenience operator for LDAP inequality.
         *
         * Writing either
         * \code
         * LDAPProp("attr") != "val"
         * \endcode
         * or
         * \code
         * !(LDAPProp("attr") == "val")
         * \endcode
         * leads to the same string "(!(attr=val))".
         *
         * @param s A type convertible to std::string.
         * @return A LDAP expression object.
         *
         * @{
         */
        LDAPPropExpr operator!=(std::string const& s) const;
        LDAPPropExpr operator!=(cppmicroservices::Any const& s) const;
        template <class T>
        LDAPPropExpr
        operator!=(T const& s) const
        {
            std::stringstream ss;
            ss << s;
            return operator!=(ss.str());
        }
        /// @}

        /**
         * \addtogroup gr_ldap
         * LDAP greater or equal '>='
         *
         * @param s A type convertible to std::string.
         * @return A LDAP expression object.
         *
         * @{
         */
        LDAPPropExpr operator>=(std::string const& s) const;
        LDAPPropExpr operator>=(cppmicroservices::Any const& s) const;
        template <class T>
        LDAPPropExpr
        operator>=(T const& s) const
        {
            std::stringstream ss;
            ss << s;
            return operator>=(ss.str());
        }
        /// @}

        /**
         * \addtogroup gr_ldap
         * LDAP less or equal '<='
         *
         * @param s A type convertible to std::string.
         * @return A LDAP expression object.
         *
         * @{
         */
        LDAPPropExpr operator<=(std::string const& s) const;
        LDAPPropExpr operator<=(cppmicroservices::Any const& s) const;
        template <class T>
        LDAPPropExpr
        operator<=(T const& s) const
        {
            std::stringstream ss;
            ss << s;
            return operator<=(ss.str());
        }
        /// @}

        /**
         * \addtogroup gr_ldap
         * LDAP approximation '~='
         *
         * @param s A type convertible to std::string.
         * @return A LDAP expression object.
         *
         * @{
         */
        LDAPPropExpr Approx(std::string const& s) const;
        LDAPPropExpr Approx(cppmicroservices::Any const& s) const;
        template <class T>
        LDAPPropExpr
        Approx(T const& s) const
        {
            std::stringstream ss;
            ss << s;
            return Approx(ss.str());
        }
        /// @}

      private:
        LDAPProp& operator=(LDAPProp const&) = delete;

        std::string m_property;
    };
} // namespace cppmicroservices

#ifdef _MSC_VER
#    pragma warning(pop)
#endif

/**
 * \ingroup MicroServicesUtils
 * \ingroup gr_ldap
 *
 * LDAP logical and '&'
 *
 * @param left A LDAP expression.
 * @param right A LDAP expression.
 * @return A LDAP expression
 */
US_Framework_EXPORT cppmicroservices::LDAPPropExpr operator&&(cppmicroservices::LDAPPropExpr const& left,
                                                              cppmicroservices::LDAPPropExpr const& right);

/**
 * \ingroup MicroServicesUtils
 * \ingroup gr_ldap
 *
 * LDAP logical or '|'
 *
 * @param left A LDAP expression.
 * @param right A LDAP expression.
 * @return A LDAP expression
 */
US_Framework_EXPORT cppmicroservices::LDAPPropExpr operator||(cppmicroservices::LDAPPropExpr const& left,
                                                              cppmicroservices::LDAPPropExpr const& right);

#endif // CPPMICROSERVICES_LDAPPROP_H
