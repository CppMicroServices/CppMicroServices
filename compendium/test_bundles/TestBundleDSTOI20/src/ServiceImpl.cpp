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
#include "ServiceImpl.hpp"

#include "cppmicroservices/GetBundleContext.h"

#include <cassert>
#include <iostream>

namespace sample {
ServiceComponent::ServiceComponent()
{
  std::clog << "Before GetBundleContext()" << std::endl;
  auto context = cppmicroservices::GetBundleContext();
  std::clog << "Before GetServiceReference()" << std::endl;
  auto sRef = context.GetServiceReference<test::LifeCycleValidation>();
  std::clog << "Before GetService()" << std::endl;
  service = context.GetService<test::LifeCycleValidation>(sRef);
  std::clog << "Done..." << std::endl;
  assert(service);
}

std::shared_ptr<test::LifeCycleValidation> ServiceComponent::GetService()
{
  return service;
}

ServiceComponent::~ServiceComponent() {}
}
