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

#include "cppmicroservices/em/Event.hpp"

#include <regex>
#include <utility>

namespace cppmicroservices::service::em
{

    namespace
    {
        bool
        ValidateTopic(std::string const& topic)
        {
            std::string regexExpr = "([A-Za-z0-9_.]+)(\\/[A-Za-z0-9_.]+)*";

            return std::regex_match(topic, std::regex(regexExpr));
        }

        bool
        PropsAreEqual(AnyMap const& props1, AnyMap const& props2)
        {
            return props1.size() == props2.size() && std::equal(props1.begin(), props1.end(), props2.begin());
        }
    } // namespace

    Event::Event(std::string const& topic, EventProperties const properties) : topic(topic), properties(properties)
    {
        if (!ValidateTopic(topic))
        {
            throw std::logic_error("The topic does not match the expected format.");
        }
    }

    bool
    Event::operator==(Event const& other) const
    {
        return (topic == other.topic) && PropsAreEqual(properties, other.properties);
    }

    bool
    Event::operator!=(Event const& other) const
    {
        return !(operator==(other));
    }

    [[nodiscard]] bool
    Event::ContainsProperty(std::string const& propName) const
    {
        return properties.find(propName) != properties.end();
    }

    [[nodiscard]] Any const
    Event::GetProperty(std::string const& propName) const
    {
        auto itr = properties.find(propName);
        if (itr == properties.end())
        {
            return Any();
        }

        return itr->second;
    }

    [[nodiscard]] AnyMap const
    Event::GetProperties() const
    {
        return properties;
    }

    [[nodiscard]] std::vector<std::string>
    Event::GetPropertyNames() const
    {
        std::vector<std::string> props(properties.size());

        size_t index = 0;
        for (auto const& [key, value] : properties)
        {
            US_UNUSED(value);
            props[index++] = key;
        }

        return props;
    }

    [[nodiscard]] std::string
    Event::GetTopic() const
    {
        return topic;
    }

    [[nodiscard]] bool
    Event::Matches(LDAPFilter const& filter) const
    {
        return filter.Match(properties);
    }

} // namespace cppmicroservices::service::em