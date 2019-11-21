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
#ifndef COMPONENTINFO_HPP
#define COMPONENTINFO_HPP

#include <string>
#include <vector>

namespace codegen {
namespace datamodel {

struct ReferenceInfo
{
  std::string name;
  std::string interface;
  std::string cardinality;
  std::string policy;
  std::string policy_option;
  std::string target;
  std::string scope;
};

struct ServiceInfo
{
  std::string scope;
  std::vector<std::string> interfaces;
};

struct ComponentInfo
{
  std::string name;
  std::string implClassName;
  bool injectReferences;
  ServiceInfo service;
  std::vector<ReferenceInfo> references;
};

// These functions return the string representations for the data model
// that's understood by the code-generator
std::string GetComponentNameStr(const ComponentInfo& compInfo);
std::string GetServiceInterfacesStr(const ServiceInfo& compInfo);
std::string GetCtorInjectedRefTypes(const ComponentInfo& compInfo);
std::string GetCtorInjectedRefNames(const ComponentInfo& compInfo);
std::string GetReferenceBinderStr(const ReferenceInfo& ref, bool injectReferences);

} // namespace datamodel
} // namespace codegen

#endif // COMPONENTINFO_HPP
