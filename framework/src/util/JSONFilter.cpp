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
#include "cppmicroservices/Constants.h"

#include "cppmicroservices/JSONFilter.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/ServiceReference.h"

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath.hpp>

#include "Properties.h"
#include "ServiceReferenceBasePrivate.h"

namespace cppmicroservices
{

    JSONFilter::JSONFilter() : filter_str(std::string()) {}

    JSONFilter::JSONFilter(std::string const& filter) : filter_str(filter)
    {
        std::error_code ec;
        auto expr = jsoncons::jmespath::jmespath_expression<jsoncons::json>::compile(filter, ec);
        if (ec)
        {
            throw std::invalid_argument(jsoncons::jmespath::jmespath_error(ec).what());
        }
    }

    JSONFilter::JSONFilter(JSONFilter const&) = default;

    JSONFilter::~JSONFilter() = default;

    JSONFilter::operator bool() const { return !filter_str.empty(); }

    bool
    JSONFilter::Match(ServiceReferenceBase const& reference) const
    {
        if (filter_str.empty())
        {
            return false;
        }

        auto const& props_handle = reference.d.load()->GetProperties();
        jsoncons::json js = props_handle->JSON_unlocked();
        jsoncons::json result = jsoncons::jmespath::search(js, filter_str);

        return (result.is_bool() ? result.as_bool() : false);
    }

    bool
    JSONFilter::Match(Bundle const& bundle) const
    {
        if (filter_str.empty())
        {
            return false;
        }

        auto props = Properties(bundle.GetHeaders());
        jsoncons::json js = PropertiesHandle(props, false)->JSON_unlocked();
        jsoncons::json result = jsoncons::jmespath::search(js, filter_str);

        return (result.is_bool() ? result.as_bool() : false);
    }

    bool
    JSONFilter::Match(AnyMap const& dictionary) const
    {
        if (filter_str.empty())
        {
            return false;
        }

        jsoncons::json js = PropertiesHandle(Properties(dictionary), false)->JSON_unlocked();
        jsoncons::json result = jsoncons::jmespath::search(js, filter_str);

        return (result.is_bool() ? result.as_bool() : false);
    }

    std::string
    JSONFilter::ToString() const
    {
        return filter_str;
    }

    bool
    JSONFilter::operator==(JSONFilter const& other) const
    {
        return (this->ToString() == other.ToString());
    }

    JSONFilter& JSONFilter::operator=(JSONFilter const& filter) = default;

    std::ostream&
    operator<<(std::ostream& os, JSONFilter const& filter)
    {
        return os << filter.ToString();
    }
} // namespace cppmicroservices
