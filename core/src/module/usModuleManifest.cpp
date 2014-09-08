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

#include "usModuleManifest_p.h"

#include "jsoncpp.h"

#include <stdexcept>

US_BEGIN_NAMESPACE

namespace {

  typedef std::map<std::string, Any> AnyMap;
  typedef std::vector<Any> AnyVector;

  void ParseJsonObject(const Json::Value& jsonObject, AnyMap& anyMap);
  void ParseJsonArray(const Json::Value& jsonArray, AnyVector& anyVector);

  Any ParseJsonValue(const Json::Value& jsonValue)
  {
    if (jsonValue.isObject())
    {
      Any any = AnyMap();
      ParseJsonObject(jsonValue, ref_any_cast<AnyMap>(any));
      return any;
    }
    else if (jsonValue.isArray())
    {
      Any any = AnyVector();
      ParseJsonArray(jsonValue, ref_any_cast<AnyVector>(any));
      return any;
    }
    else if (jsonValue.isString())
    {
      return Any(jsonValue.asString());
    }
    else if (jsonValue.isBool())
    {
      return Any(jsonValue.asBool());
    }
    else if (jsonValue.isDouble())
    {
      return Any(jsonValue.asDouble());
    }
    else if (jsonValue.isIntegral())
    {
      return Any(jsonValue.asInt());
    }

    return Any();
  }

  void ParseJsonObject(const Json::Value& jsonObject, AnyMap& anyMap)
  {
    for (Json::Value::const_iterator it = jsonObject.begin();
         it != jsonObject.end(); ++it)
    {
      const Json::Value& jsonValue = *it;
      Any anyValue = ParseJsonValue(jsonValue);
      if (!anyValue.Empty())
      {
        anyMap.insert(std::make_pair(it.memberName(), anyValue));
      }
    }
  }

  void ParseJsonArray(const Json::Value& jsonArray, AnyVector& anyVector)
  {
    for (Json::Value::const_iterator it = jsonArray.begin();
         it != jsonArray.end(); ++it)
    {
      const Json::Value& jsonValue = *it;
      Any anyValue = ParseJsonValue(jsonValue);
      if (!anyValue.Empty())
      {
        anyVector.push_back(anyValue);
      }
    }
  }

}

ModuleManifest::ModuleManifest()
{
}

void ModuleManifest::Parse(std::istream& is)
{
  Json::Value root;
  Json::Reader jsonReader(Json::Features::strictMode());
  if (!jsonReader.parse(is, root, false))
  {
    throw std::runtime_error(jsonReader.getFormattedErrorMessages());
  }

  if (!root.isObject())
  {
    throw std::runtime_error("The Json root element must be an object.");
  }

  ParseJsonObject(root, m_Properties);
}

bool ModuleManifest::Contains(const std::string& key) const
{
  return m_Properties.count(key) > 0;
}

Any ModuleManifest::GetValue(const std::string& key) const
{
  AnyMap::const_iterator iter = m_Properties.find(key);
  if (iter != m_Properties.end())
  {
    return iter->second;
  }
  return Any();
}

std::vector<std::string> ModuleManifest::GetKeys() const
{
  std::vector<std::string> keys;
  for (AnyMap::const_iterator iter = m_Properties.begin();
       iter != m_Properties.end(); ++iter)
  {
    keys.push_back(iter->first);
  }
  return keys;
}

void ModuleManifest::SetValue(const std::string& key, const Any& value)
{
  m_Properties[key] = value;
}

US_END_NAMESPACE
