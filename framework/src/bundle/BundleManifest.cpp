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

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include <stdexcept>
#include <typeinfo>

namespace {

using AnyOrderedMap = std::map<std::string, cppmicroservices::Any>;
using AnyMap = cppmicroservices::AnyMap;
using AnyVector = std::vector<cppmicroservices::Any>;

/**
 * recursively copy the content of headers into deprecated. "headers" is an AnyMap, which
 * stores any hierarchical values in AnyMaps also. The purpose of this function is to
 * store the data in a std::map hierarchy instead.
 */
void copy_deprecated_properties(const AnyMap& headers
                                  , AnyOrderedMap& deprecated)
{
  for (auto const& h : headers) {
    if (typeid(AnyMap) == h.second.Type()) {
      // recursively copy the anymap to a std::map and store in deprecated.
      AnyOrderedMap deprecated_headers;
      copy_deprecated_properties(cppmicroservices::any_cast<AnyMap>(h.second)
                                 , deprecated_headers);
      deprecated.emplace(h.first, std::move(deprecated_headers));
    }
    else {
      deprecated.emplace(h.first, h.second);
    }
  }
}

}

namespace cppmicroservices {

namespace {

void ParseJsonObject(const rapidjson::Value& jsonObject, AnyMap& anyMap);
void ParseJsonObject(const rapidjson::Value& jsonObject, AnyOrderedMap& anyMap);
void ParseJsonArray(const rapidjson::Value& jsonArray,
                    AnyVector& anyVector,
                    bool ci);


Any ParseJsonValue(const rapidjson::Value& jsonValue, bool ci)
{
  if (jsonValue.IsObject()) {
    if (ci) {
      Any any = AnyMap(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
      ParseJsonObject(jsonValue, ref_any_cast<AnyMap>(any));
      return any;
    } else {
      Any any = AnyOrderedMap();
      ParseJsonObject(jsonValue, ref_any_cast<AnyOrderedMap>(any));
      return any;
    }
  } else if (jsonValue.IsArray()) {
    Any any = AnyVector();
    ParseJsonArray(jsonValue, ref_any_cast<AnyVector>(any), ci);
    return any;
  } else if (jsonValue.IsString()) {
    // We do not support attribute localization yet, so we just
    // always remove the leading '%' character.
    std::string val = jsonValue.GetString();
    if (!val.empty() && val[0] == '%')
      val = val.substr(1);

    return Any(val);
  } else if (jsonValue.IsBool()) {
    return Any(jsonValue.GetBool());
  } else if (jsonValue.IsInt()) {
    return Any(jsonValue.GetInt());
  } else if (jsonValue.IsDouble()) {
    return Any(jsonValue.GetDouble());
  }

  return Any();
}

void ParseJsonObject(const rapidjson::Value& jsonObject, AnyOrderedMap& anyMap)
{
  for (const auto& m : jsonObject.GetObject()) {
    Any anyValue = ParseJsonValue(m.value, false);
    if (!anyValue.Empty()) {
      anyMap.emplace(m.name.GetString(), std::move(anyValue));
    }
  }
}

void ParseJsonObject(const rapidjson::Value& jsonObject, AnyMap& anyMap)
{
  for (const auto& m : jsonObject.GetObject()) {
    Any anyValue = ParseJsonValue(m.value, true);
    if (!anyValue.Empty()) {
      anyMap.emplace(m.name.GetString(), std::move(anyValue));
    }
  }
}

void ParseJsonArray(const rapidjson::Value& jsonArray, AnyVector& anyVector, bool ci)
{
  for (const auto& jsonValue : jsonArray.GetArray()) {
    Any anyValue = ParseJsonValue(jsonValue, ci);
    if (!anyValue.Empty()) {
      anyVector.emplace_back(std::move(anyValue));
    }
  }
}

}

BundleManifest::BundleManifest()
  : m_Headers(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS)
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

  
  ParseJsonObject(root, m_Headers);
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
  if (m_Headers.cend() != iter)
  {
    return iter->second;
  }
  return Any();
}

void BundleManifest::CopyDeprecatedProperties() const
{
  std::call_once(m_DidCopyDeprecatedProperties
                 , [&]() { copy_deprecated_properties(m_Headers, m_PropertiesDeprecated); });
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
