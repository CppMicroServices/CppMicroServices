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

#include "usServiceRegistrationPrivate.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4355)
#endif

US_BEGIN_NAMESPACE

ServiceRegistrationPrivate::ServiceRegistrationPrivate(
  ModulePrivate* module, US_BASECLASS_NAME* service,
  const ServiceProperties& props)
  : ref(0), service(service), module(module), reference(this),
    properties(props), available(true), unregistering(false)
{
  // The reference counter is initialized to 0 because it will be
  // incremented by the "reference" member.
}

ServiceRegistrationPrivate::~ServiceRegistrationPrivate()
{

}

bool ServiceRegistrationPrivate::IsUsedByModule(Module* p)
{
  return dependents.find(p) != dependents.end();
}

US_BASECLASS_NAME* ServiceRegistrationPrivate::GetService()
{
  return service;
}

US_END_NAMESPACE

#ifdef _MSC_VER
#pragma warning(pop)
#endif
