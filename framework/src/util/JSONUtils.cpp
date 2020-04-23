/**
 * @file      JSONUtils.cpp
 * @copyright Copyright 2020 MathWorks, Inc. 
 */ 

#include "JSONUtils.h"
#include "cppmicroservices/AnyMap.h"
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

namespace cppmicroservices { namespace json {

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

}} // namespaces
