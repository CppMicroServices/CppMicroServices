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

#include "cppmicroservices/LDAPProp.h"

#include <stdexcept>
#include <utility>

namespace cppmicroservices
{

    LDAPPropExpr::LDAPPropExpr() : m_ldapExpr() {}

    LDAPPropExpr::LDAPPropExpr(std::string expr) : m_ldapExpr(std::move(expr)) {}

    LDAPPropExpr&
    LDAPPropExpr::operator!()
    {
        if (m_ldapExpr.empty())
            return *this;

        m_ldapExpr = "(!" + m_ldapExpr + ")";
        return *this;
    }

    LDAPPropExpr::operator std::string() const { return m_ldapExpr; }

    bool
    LDAPPropExpr::IsNull() const
    {
        return m_ldapExpr.empty();
    }

    LDAPPropExpr&
    LDAPPropExpr::operator|=(LDAPPropExpr const& right)
    {
        m_ldapExpr = (*this || right).m_ldapExpr;
        return *this;
    }

    LDAPPropExpr&
    LDAPPropExpr::operator&=(LDAPPropExpr const& right)
    {
        m_ldapExpr = (*this && right).m_ldapExpr;
        return *this;
    }

    LDAPProp::LDAPProp(std::string property) : m_property(std::move(property))
    {
        if (m_property.empty())
            throw std::invalid_argument("property must not be empty");
    }

    LDAPPropExpr
    LDAPProp::operator==(std::string const& s) const
    {
        if (s.empty())
            return LDAPPropExpr(s);
        return LDAPPropExpr("(" + m_property + "=" + s + ")");
    }

    LDAPPropExpr
    LDAPProp::operator==(cppmicroservices::Any const& any) const
    {
        return operator==(any.ToString());
    }

    LDAPPropExpr
    LDAPProp::operator==(bool b) const
    {
        return operator==(b ? std::string("true") : std::string("false"));
    }

    LDAPProp::operator LDAPPropExpr() const { return LDAPPropExpr("(" + m_property + "=*)"); }

    LDAPPropExpr
    LDAPProp::operator!() const
    {
        return LDAPPropExpr("(!(" + m_property + "=*))");
    }

    LDAPPropExpr
    LDAPProp::operator!=(std::string const& s) const
    {
        if (s.empty())
            return LDAPPropExpr(s);
        return LDAPPropExpr("(!(" + m_property + "=" + s + "))");
    }

    LDAPPropExpr
    LDAPProp::operator!=(cppmicroservices::Any const& any) const
    {
        return operator!=(any.ToString());
    }

    LDAPPropExpr
    LDAPProp::operator>=(std::string const& s) const
    {
        if (s.empty())
            return LDAPPropExpr(s);
        return LDAPPropExpr("(" + m_property + ">=" + s + ")");
    }

    LDAPPropExpr
    LDAPProp::operator>=(cppmicroservices::Any const& any) const
    {
        return operator>=(any.ToString());
    }

    LDAPPropExpr
    LDAPProp::operator<=(std::string const& s) const
    {
        if (s.empty())
            return LDAPPropExpr(s);
        return LDAPPropExpr("(" + m_property + "<=" + s + ")");
    }

    LDAPPropExpr
    LDAPProp::operator<=(cppmicroservices::Any const& any) const
    {
        return operator<=(any.ToString());
    }

    LDAPPropExpr
    LDAPProp::Approx(std::string const& s) const
    {
        if (s.empty())
            return LDAPPropExpr(s);
        return LDAPPropExpr("(" + m_property + "~=" + s + ")");
    }

    LDAPPropExpr
    LDAPProp::Approx(cppmicroservices::Any const& any) const
    {
        return Approx(any.ToString());
    }
} // namespace cppmicroservices

cppmicroservices::LDAPPropExpr
operator&&(cppmicroservices::LDAPPropExpr const& left, cppmicroservices::LDAPPropExpr const& right)
{
    if (left.IsNull())
        return right;
    if (right.IsNull())
        return left;
    return cppmicroservices::LDAPPropExpr("(&" + static_cast<std::string>(left) + static_cast<std::string>(right)
                                          + ")");
}

cppmicroservices::LDAPPropExpr
operator||(cppmicroservices::LDAPPropExpr const& left, cppmicroservices::LDAPPropExpr const& right)
{
    if (left.IsNull())
        return right;
    if (right.IsNull())
        return left;
    return cppmicroservices::LDAPPropExpr("(|" + static_cast<std::string>(left) + static_cast<std::string>(right)
                                          + ")");
}
