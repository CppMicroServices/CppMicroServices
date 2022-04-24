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

#include "cppmicroservices/em/EMEvent.hpp"

#include <regex>

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

EMEvent::EMEvent(const std::string& topic, const EventProperties properties)
  : topic(topic)
  , properties(properties)
{
  if (!ValidateTopic(topic)) {
    throw std::logic_error("The topic does not match the expected format.");
  }
}

bool EMEvent::operator==(const EMEvent& other) const
{
  return (topic == other.topic) && PropsAreEqual(properties, other.properties);
}

bool EMEvent::operator!=(const EMEvent& other) const
{
  return !(operator==(other));
}

bool EMEvent::ContainsProperty(const std::string& propName) const
{
  return properties.find(propName) != properties.end();
}

const Any EMEvent::GetProperty(const std::string& propName) const
{
  auto itr = properties.find(propName);
  if (itr == properties.end()) {
    return Any();
  }

  return itr->second;
}

const AnyMap EMEvent::GetProperties() const
{
  return properties;
}

std::vector<std::string> EMEvent::GetPropertyNames() const
{
  std::vector<std::string> props(properties.size());

  size_t index = 0;
  for (const auto& [key, _] : properties) {
    props[index++] = key;
  }

  return props;
}

std::string EMEvent::GetTopic() const
{
  return topic;
}

bool EMEvent::Matches(const LDAPFilter& filter) const
{
  return filter.Match(properties);
}

}
}
}
