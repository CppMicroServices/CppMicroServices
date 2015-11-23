/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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

namespace us {

ServiceRegistrationBasePrivate::ServiceRegistrationBasePrivate(
  BundlePrivate* bundle, const InterfaceMapConstPtr& service,
  const ServicePropertiesImpl& props)
  : ref(0), service(service), bundle(bundle), reference(this),
    properties(props), available(true), unregistering(false)
{
  // The reference counter is initialized to 0 because it will be
  // incremented by the "reference" member.
}

ServiceRegistrationBasePrivate::~ServiceRegistrationBasePrivate()
{

}

bool ServiceRegistrationBasePrivate::IsUsedByBundle(const std::shared_ptr<Bundle>& bundle) const
{
  return (dependents.find(bundle) != dependents.end()) ||
    (prototypeServiceInstances.find(bundle) != prototypeServiceInstances.end());
}

const InterfaceMapConstPtr& ServiceRegistrationBasePrivate::GetInterfaces() const
{
  return service;
}

std::shared_ptr<void> ServiceRegistrationBasePrivate::GetService(const std::string& interfaceId) const
{
  if (interfaceId.empty() && service->size() > 0)
  {
    return service->begin()->second;
  }

  InterfaceMap::const_iterator iter = service->find(interfaceId);
  if (iter != service->end())
  {
    return iter->second;
  }
  return NULL;
}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
