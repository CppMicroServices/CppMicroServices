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

#include "cppmicroservices/LDAPFilter.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/ServiceReference.h"

#include "LDAPExpr.h"
#include "Properties.h"
#include "PropsCheck.h"
#include "ServiceReferenceBasePrivate.h"
#include "Utils.h"

#include <stdexcept>

namespace cppmicroservices
{

    class LDAPFilterData
    {
      public:
        LDAPFilterData() : ldapExpr() {}

        LDAPFilterData(std::string const& filter) : ldapExpr(filter) {}

        LDAPFilterData(LDAPFilterData const&) = default;

        LDAPExpr ldapExpr;
    };

    LDAPFilter::LDAPFilter() : d(nullptr) {}

    LDAPFilter::LDAPFilter(std::string const& filter) : d(nullptr)
    {
        try
        {
            d = std::make_shared<LDAPFilterData>(filter);
        }
        catch (std::exception const& e)
        {
            throw std::invalid_argument(e.what());
        }
    }

    LDAPFilter::LDAPFilter(LDAPFilter const&) = default;

    LDAPFilter::~LDAPFilter() = default;

    LDAPFilter::operator bool() const { return d != nullptr; }

    bool
    LDAPFilter::Match(ServiceReferenceBase const& reference) const
    {
        return ((d) ? d->ldapExpr.Evaluate(reference.d.Load()->GetProperties(), false) : false);
    }

    // This function has been modified to call the LDAPExpr::Evaluate() function which takes
    // an AnyMap rather than a PropertiesHandle to optimize the code. Constructing a Properties
    // object is much slower (requiring a copy) than simply using the AnyMap directly.
    bool
    LDAPFilter::Match(Bundle const& bundle) const
    {
        if (d)
        {
            auto const& headers = bundle.GetHeaders();

            if (headers.GetType() != AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS)
            {
                props_check::ValidateAnyMap(headers);
            }

            return d->ldapExpr.Evaluate(headers, false);
        }
        else
        {
            return false;
        }
    }

    // This function has been modified to call the LDAPExpr::Evaluate() function which takes
    // an AnyMap rather than a PropertiesHandle to optimize the code. Constructing a Properties
    // object is much slower (requiring a copy) than simply using the AnyMap directly.
    bool
    LDAPFilter::Match(AnyMap const& dictionary) const
    {
        if (d)
        {
            if (dictionary.GetType() != AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS)
            {
                props_check::ValidateAnyMap(dictionary);
            }

            return d->ldapExpr.Evaluate(dictionary, false);
        }
        else
        {
            return false;
        }
    }

    // This function has been modified to call the LDAPExpr::Evaluate() function which takes
    // an AnyMap rather than a PropertiesHandle to optimize the code. Constructing a Properties
    // object is much slower (requiring a copy) than simply using the AnyMap directly.
    bool
    LDAPFilter::MatchCase(AnyMap const& dictionary) const
    {
        if (d)
        {
            if (dictionary.GetType() != AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS)
            {
                props_check::ValidateAnyMap(dictionary);
            }

            return d->ldapExpr.Evaluate(dictionary, true);
        }
        else
        {
            return false;
        }
    }

    std::string
    LDAPFilter::ToString() const
    {
        return ((d) ? d->ldapExpr.ToString() : std::string());
    }

    bool
    LDAPFilter::operator==(LDAPFilter const& other) const
    {
        return (this->ToString() == other.ToString());
    }

    LDAPFilter& LDAPFilter::operator=(LDAPFilter const& filter) = default;

    std::ostream&
    operator<<(std::ostream& os, LDAPFilter const& filter)
    {
        return os << filter.ToString();
    }
} // namespace cppmicroservices
