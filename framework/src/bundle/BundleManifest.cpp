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

#include "BundleManifest.h"
#include "Utils.h"
#include "cppmicroservices/Any.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include <stdexcept>
#include <typeinfo>

namespace {

using namespace cppmicroservices;

using AnyOrderedMap = std::map<std::string, Any>;
using AnyVector = std::vector<Any>;

/**
 * recursively copy the content of headers into deprecated. "headers" is an AnyMap, which
 * stores any hierarchical values in AnyMaps also. The purpose of this function is to
 * store the data in a std::map hierarchy instead.
 */
void copy_deprecated_properties(const AnyMap& headers,
                                AnyOrderedMap& deprecated)
{
  for (auto const& h : headers) {
    if (typeid(AnyMap) == h.second.Type()) {
      // recursively copy the anymap to a std::map and store in deprecated.
      AnyOrderedMap deprecated_headers;
      copy_deprecated_properties(cppmicroservices::any_cast<AnyMap>(h.second),
                                 deprecated_headers);
      deprecated.emplace(h.first, std::move(deprecated_headers));
    } else {
      deprecated.emplace(h.first, h.second);
    }
  }
}

}

namespace cppmicroservices {

BundleManifest::BundleManifest()
  : m_Headers(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS)
{}

BundleManifest::BundleManifest(const AnyMap& m)
  : m_Headers(m)
{}

void BundleManifest::Parse(std::istream& is)
{
  rapidjson::IStreamWrapper jsonStream(is);
  rapidjson::Document root;
  if (root.ParseStream(jsonStream).HasParseError()) {
    throw std::runtime_error(rapidjson::GetParseError_En(root.GetParseError()));
  }

  if (!root.IsObject()) {
    throw std::runtime_error("The Json root element must be an object.");
  }

  json::ParseObject(root, m_Headers);
}

const AnyMap& BundleManifest::GetHeaders() const
{
  return m_Headers;
}

bool BundleManifest::Contains(const std::string& key) const
{
  return m_Headers.count(key) > 0;
}

Any BundleManifest::GetValue(const std::string& key) const
{
  auto iter = m_Headers.find(key);
  if (m_Headers.cend() != iter) {
    return iter->second;
  }
  return Any();
}

void BundleManifest::CopyDeprecatedProperties() const
{
  std::call_once(m_DidCopyDeprecatedProperties, [&]() {
    copy_deprecated_properties(m_Headers, m_PropertiesDeprecated);
  });
}

Any BundleManifest::GetValueDeprecated(const std::string& key) const
{
  CopyDeprecatedProperties();
  auto iter = m_PropertiesDeprecated.find(key);
  if (m_PropertiesDeprecated.cend() != iter) {
    return iter->second;
  }
  return Any();
}

std::vector<std::string> BundleManifest::GetKeysDeprecated() const
{
  CopyDeprecatedProperties();
  std::vector<std::string> keys;
  for (AnyMap::const_iterator iter = m_PropertiesDeprecated.cbegin();
       iter != m_PropertiesDeprecated.cend();
       ++iter) {
    keys.push_back(iter->first);
  }
  return keys;
}

std::map<std::string, Any> BundleManifest::GetPropertiesDeprecated() const
{
  CopyDeprecatedProperties();
  return m_PropertiesDeprecated;
}

}
