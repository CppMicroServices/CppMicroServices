/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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

#include "usLDAPProp.h"

US_BEGIN_NAMESPACE

LDAPPropExpr::LDAPPropExpr(const std::string& expr)
  : m_ldapExpr(expr)
{}

LDAPPropExpr& LDAPPropExpr::operator!()
{
  m_ldapExpr = "(!" + m_ldapExpr + ")";
  return *this;
}

LDAPPropExpr::operator std::string() const
{
  return m_ldapExpr;
}


LDAPProp::LDAPProp(const std::string& property)
  : m_property(property)
{}

LDAPPropExpr LDAPProp::operator==(const std::string& s) const
{
  return LDAPPropExpr("(" + m_property + "=" + s + ")");
}

LDAPPropExpr LDAPProp::operator==(const us::Any& any) const
{
  return operator==(any.ToString());
}

LDAPProp::operator LDAPPropExpr () const
{
  return LDAPPropExpr("(" + m_property + "=*)");
}

LDAPPropExpr LDAPProp::operator!() const
{
  return LDAPPropExpr("(!(" + m_property + "=*))");
}

LDAPPropExpr LDAPProp::operator!=(const std::string& s) const
{
  return LDAPPropExpr("(!(" + m_property + "=" + s + "))");
}

LDAPPropExpr LDAPProp::operator!=(const us::Any& any) const
{
  return operator!=(any.ToString());
}

LDAPPropExpr LDAPProp::operator>=(const std::string& s) const
{
  return LDAPPropExpr("(" + m_property + ">=" + s + ")");
}

LDAPPropExpr LDAPProp::operator>=(const us::Any& any) const
{
  return operator>=(any.ToString());
}

LDAPPropExpr LDAPProp::operator<=(const std::string& s) const
{
  return LDAPPropExpr("(" + m_property + "<=" + s + ")");
}

LDAPPropExpr LDAPProp::operator<=(const us::Any& any) const
{
  return operator<=(any.ToString());
}

LDAPPropExpr LDAPProp::Approx(const std::string& s) const
{
  return LDAPPropExpr("(" + m_property + "~=" + s + ")");
}

LDAPPropExpr LDAPProp::Approx(const us::Any& any) const
{
  return Approx(any.ToString());
}


US_END_NAMESPACE

us::LDAPPropExpr operator&&(const us::LDAPPropExpr& left, const us::LDAPPropExpr& right)
{
  return us::LDAPPropExpr("(&" + static_cast<std::string>(left) + static_cast<std::string>(right) + ")");
}

us::LDAPPropExpr operator||(const us::LDAPPropExpr& left, const us::LDAPPropExpr& right)
{
  return us::LDAPPropExpr("(|" + static_cast<std::string>(left) + static_cast<std::string>(right) + ")");
}
