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

#ifndef USMODULEMANIFEST_P_H
#define USMODULEMANIFEST_P_H

#include "usAny.h"

#include "json_p.h"

US_BEGIN_NAMESPACE

class ModuleManifest
{
public:

  ModuleManifest();

  void Parse(std::istream& is);

  bool Contains(const std::string& key) const;

  Any GetValue(const std::string& key) const;

  std::vector<std::string> GetKeys() const;

  void SetValue(const std::string& key, const Any& value);

private:

  typedef std::map<std::string, Any> AnyMap;
  typedef std::vector<Any> AnyVector;

  Any ParseJsonValue(const Json::Value& jsonValue);

  void ParseJsonObject(const Json::Value& jsonObject, AnyMap& anyMap);
  void ParseJsonArray(const Json::Value& jsonArray, AnyVector& anyVector);

  AnyMap m_Properties;
};

US_END_NAMESPACE

#endif // USMODULEMANIFEST_P_H
