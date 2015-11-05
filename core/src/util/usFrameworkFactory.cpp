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

#include "usFrameworkFactory.h"
#include "usFramework.h"
#include "usBundleInfo.h"
#include "usBundleUtils.h"
#include "usFrameworkPrivate.h"

namespace us {

FrameworkFactory::FrameworkFactory(void)
{
}

FrameworkFactory::~FrameworkFactory(void)
{
}

std::shared_ptr<Framework> FrameworkFactory::NewFramework(const std::map<std::string, Any>& configuration)
{
  BundleInfo bundleInfo(US_CORE_FRAMEWORK_NAME);

  std::shared_ptr<Framework>(FrameworkFactory::*newFWFncPtr)(const std::map<std::string, Any>&) = &FrameworkFactory::NewFramework;
  void* newFramework = nullptr;
  std::memcpy(&newFramework, &newFWFncPtr, sizeof(void*));
  bundleInfo.location = BundleUtils::GetLibraryPath(newFramework);
  bundleInfo.id = 0;

  std::shared_ptr<Framework> fw(new Framework(bundleInfo, configuration));
  fw->d->Init(&static_cast<FrameworkPrivate*>(fw->d.get())->coreBundleContext);
  return fw;
}

std::shared_ptr<Framework> FrameworkFactory::NewFramework(void)
{
  return NewFramework(std::map<std::string, Any>());
}

}
