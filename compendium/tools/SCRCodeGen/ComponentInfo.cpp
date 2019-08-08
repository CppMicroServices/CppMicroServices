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

#include <regex>
#include <sstream>

#include "ComponentInfo.hpp"

namespace codegen {
namespace datamodel {

std::string GetComponentNameStr(const ComponentInfo& compInfo)
{
  const auto name = compInfo.name.empty() ? compInfo.implClassName : compInfo.name;
  return std::regex_replace(name, std::regex("(::)"), "_");
}

std::string GetServiceInterfacesStr(const ServiceInfo& serviceInfo)
{
  auto& interfaces = serviceInfo.interfaces;
  if (interfaces.empty())
  {
    return "";
  }
  std::ostringstream strstream;
  std::copy(std::begin(interfaces), std::end(interfaces) - 1,
            std::ostream_iterator<std::string>(strstream, ", "));
  strstream << interfaces.back();
  return strstream.str();
}

std::string GetReferencesStr(const ComponentInfo& compInfo)
{
  std::string result;
  const auto delim = ", ";
  auto sep = "";
  for (const auto& reference :  compInfo.references)
  {
    result += (sep + reference.interface);
    sep = delim;
  }
  return result;
}

std::string GetInjectReferencesStr(const ComponentInfo& compInfo)
{
  return compInfo.injectReferences ? "std::true_type" : "std::false_type";
}

std::string GetReferenceBinderStr(const ReferenceInfo& ref)
{
  auto isStatic = (ref.policy == "static");
  std::string binderObjStr = "std::make_shared<";
  binderObjStr += isStatic ? "Static" : "Dynamic";
  binderObjStr += "Binder<{0}, ";
  binderObjStr += ref.interface;
  binderObjStr += ">>(\"" + ref.name + "\"";
  binderObjStr += isStatic ? "" : ", &{0}::Bind" + ref.name + ", &{0}::Unbind" + ref.name;
  binderObjStr += ")";
  return binderObjStr;
}
} // namespace datamodel
} // namespace codegen

