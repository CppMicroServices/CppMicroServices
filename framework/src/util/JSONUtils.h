/**
 * @file      JSONUtils.hpp
 * @copyright Copyright 2020 MathWorks, Inc. 
 */ 

#ifndef JSONUTILS_HPP
#define JSONUTILS_HPP

#include <rapidjson/document.h>
#include "cppmicroservices/Any.h"
#include "cppmicroservices/AnyMap.h"

namespace cppmicroservices { namespace json {

using AnyOrderedMap = std::map<std::string, cppmicroservices::Any>;
using AnyMap = cppmicroservices::AnyMap;
using AnyVector = std::vector<cppmicroservices::Any>;

US_Framework_EXPORT void ParseJsonObject(const rapidjson::Value& jsonObject, AnyMap& anyMap);
US_Framework_EXPORT void ParseJsonObject(const rapidjson::Value& jsonObject, AnyOrderedMap& anyMap);
US_Framework_EXPORT void ParseJsonArray(const rapidjson::Value& jsonArray,
                    AnyVector& anyVector,
                    bool ci);

}}
#endif
