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

#include "usServiceRegistrationBasePrivate.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4355)
#endif

US_BEGIN_NAMESPACE

ServiceRegistrationBasePrivate::ServiceRegistrationBasePrivate(
  ModulePrivate* module, const InterfaceMap& service,
  const ServicePropertiesImpl& props)
  : ref(0), service(service), module(module), reference(this),
    properties(props), available(true), unregistering(false)
{
  // The reference counter is initialized to 0 because it will be
  // incremented by the "reference" member.
}

ServiceRegistrationBasePrivate::~ServiceRegistrationBasePrivate()
{

}

bool ServiceRegistrationBasePrivate::IsUsedByModule(Module* p) const
{
  return (dependents.find(p) != dependents.end()) ||
      (prototypeServiceInstances.find(p) != prototypeServiceInstances.end());
}

const InterfaceMap& ServiceRegistrationBasePrivate::GetInterfaces() const
{
  return service;
}

void* ServiceRegistrationBasePrivate::GetService(const std::string& interfaceId) const
{
  if (interfaceId.empty() && service.size() > 0)
  {
    return service.begin()->second;
  }

  InterfaceMap::const_iterator iter = service.find(interfaceId);
  if (iter != service.end())
  {
    return iter->second;
  }
  return NULL;
}

US_END_NAMESPACE

#ifdef _MSC_VER
#pragma warning(pop)
#endif
