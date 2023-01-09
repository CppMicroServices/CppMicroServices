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

#include "cppmicroservices/JSONProp.h"

#include <stdexcept>
#include <utility>

namespace cppmicroservices
{

    JSONPropExpr::JSONPropExpr() : m_jsonExpr() {}

    JSONPropExpr::JSONPropExpr(std::string expr) : m_jsonExpr(std::move(expr)) {}

    JSONPropExpr&
    JSONPropExpr::operator!()
    {
        m_jsonExpr = "!" + m_jsonExpr;
        return *this;
    }

    JSONPropExpr::operator std::string() const { return m_jsonExpr; }

    bool
    JSONPropExpr::IsNull() const
    {
        return m_jsonExpr.empty();
    }

    JSONPropExpr&
    JSONPropExpr::operator|=(JSONPropExpr const& right)
    {
        m_jsonExpr = (*this || right).m_jsonExpr;
        return *this;
    }

    JSONPropExpr&
    JSONPropExpr::operator&=(JSONPropExpr const& right)
    {
        m_jsonExpr = (*this && right).m_jsonExpr;
        return *this;
    }

    JSONProp::JSONProp(std::string property) : m_property(std::move(property))
    {
        if (m_property.empty())
            throw std::invalid_argument("property must not be empty");
    }

    JSONPropExpr
    JSONProp::operator==(std::string const& s) const
    {
        if (s.empty())
            return JSONPropExpr(s);
        return JSONPropExpr(m_property + " == \'" + s + "\'");
    }

    JSONPropExpr
    JSONProp::operator==(cppmicroservices::Any const& any) const
    {
        return operator==(any.ToJSON());
    }

    JSONPropExpr
    JSONProp::operator==(bool b) const
    {
        return (b == true ? JSONPropExpr(m_property + " == `true`") : JSONPropExpr(m_property + " == `false`"));
    }

    JSONProp::operator JSONPropExpr() const { return operator==(true); }

    JSONPropExpr
    JSONProp::operator!() const
    {
        return JSONPropExpr("!" + m_property);
    }

    JSONPropExpr
    JSONProp::operator!=(std::string const& s) const
    {
        if (s.empty())
            return JSONPropExpr(s);
        return JSONPropExpr(m_property + "!=\'" + s + "\'");
    }

    JSONPropExpr
    JSONProp::operator!=(cppmicroservices::Any const& any) const
    {
        return operator!=(any.ToJSON());
    }

    JSONPropExpr
    JSONProp::operator>=(std::string const& s) const
    {
        throw std::logic_error("Do we need to implement this for strings ?");
    }

    JSONPropExpr
    JSONProp::operator>=(cppmicroservices::Any const& any) const
    {
        return operator>=(any.ToJSON());
    }

    JSONPropExpr
    JSONProp::operator<=(std::string const& s) const
    {
        throw std::logic_error("Do we need to implement this for strings ?");
    }

    JSONPropExpr
    JSONProp::operator<=(cppmicroservices::Any const& any) const
    {
        return operator<=(any.ToJSON());
    }

    JSONPropExpr
    JSONProp::Approx(std::string const& s) const
    {
        throw std::logic_error("Unsupported operator");
    }

    JSONPropExpr
    JSONProp::Approx(cppmicroservices::Any const& any) const
    {
        return Approx(any.ToJSON());
    }

} // namespace cppmicroservices

cppmicroservices::JSONPropExpr
operator&&(cppmicroservices::JSONPropExpr const& left, cppmicroservices::JSONPropExpr const& right)
{
    if (left.IsNull())
        return right;
    if (right.IsNull())
        return left;
    return cppmicroservices::JSONPropExpr("(" + static_cast<std::string>(left) + ") && ("
                                          + static_cast<std::string>(right) + ")");
}

cppmicroservices::JSONPropExpr
operator||(cppmicroservices::JSONPropExpr const& left, cppmicroservices::JSONPropExpr const& right)
{
    if (left.IsNull())
        return right;
    if (right.IsNull())
        return left;
    return cppmicroservices::JSONPropExpr("(" + static_cast<std::string>(left) + ") || ("
                                          + static_cast<std::string>(right) + ")");
}
