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

    /// \cond
    class US_Framework_EXPORT LDAPPropExpr
    {
      public:
        LDAPPropExpr();
        explicit LDAPPropExpr(std::string expr);

        LDAPPropExpr& operator!();

        operator std::string() const;

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
    /// \endcond

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
