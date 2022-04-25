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

namespace cppmicroservices {
namespace service {
namespace em {

namespace {
bool ValidateTopic(const std::string& topic)
{
  std::string regexExpr = "([A-Za-z0-9_.]+)(\\/[A-Za-z0-9_.]+)*";

  return std::regex_match(topic, std::regex(regexExpr));
}

bool PropsAreEqual(const AnyMap& props1, const AnyMap& props2)
{
  return props1.size() == props2.size() &&
         std::equal(props1.begin(), props1.end(), props2.begin());
}
}

Event::Event(const std::string& topic, const EventProperties properties)
  : topic(topic)
  , properties(properties)
{
  if (!ValidateTopic(topic)) {
    throw std::logic_error("The topic does not match the expected format.");
  }
}

bool Event::operator==(const Event& other) const
{
  return (topic == other.topic) && PropsAreEqual(properties, other.properties);
}

bool Event::operator!=(const Event& other) const
{
  return !(operator==(other));
}

bool Event::ContainsProperty(const std::string& propName) const
{
  return properties.find(propName) != properties.end();
}

const Any Event::GetProperty(const std::string& propName) const
{
  auto itr = properties.find(propName);
  if (itr == properties.end()) {
    return Any();
  }

  return itr->second;
}

const AnyMap Event::GetProperties() const
{
  return properties;
}

std::vector<std::string> Event::GetPropertyNames() const
{
  std::vector<std::string> props(properties.size());

  size_t index = 0;
  for (const auto& [key, value] : properties) {
    US_UNUSED(value);
    props[index++] = key;
  }

  return props;
}

std::string Event::GetTopic() const
{
  return topic;
}

bool Event::Matches(const LDAPFilter& filter) const
{
  return filter.Match(properties);
}

}
}
}
