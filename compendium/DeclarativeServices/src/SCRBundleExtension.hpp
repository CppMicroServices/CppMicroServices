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

#ifndef __SCRBUNDLEEXTENSION_HPP__
#define __SCRBUNDLEEXTENSION_HPP__

#include <memory>
#include "gtest/gtest_prod.h"
#include "cppmicroservices/BundleContext.h"
#include "ComponentRegistry.hpp"
#include "manager/ComponentManager.hpp"
#include "cppmicroservices/logservice/LogService.hpp"
#include "metadata/Util.hpp"

using cppmicroservices::logservice::LogService;

namespace cppmicroservices {
namespace scrimpl {
/**
 * The SCRBundleExtension is a helper class to load and unload Components of
 * a single bundle. It is responsible for creating a component manager for each
 * valid component description found in the bundle. On destruction, this object
 * removes and destroys the component managers created during it's construction
 */
class SCRBundleExtension
{
public:
  SCRBundleExtension(const cppmicroservices::BundleContext& bundleContext,
                     const cppmicroservices::AnyMap& scrMetadata,
                     const std::shared_ptr<ComponentRegistry>& registry,
                     const std::shared_ptr<LogService>& logger);
  SCRBundleExtension(const SCRBundleExtension&) = delete;
  SCRBundleExtension(SCRBundleExtension&&) = delete;
  SCRBundleExtension& operator=(const SCRBundleExtension&) = delete;
  SCRBundleExtension& operator=(SCRBundleExtension&&) = delete;
  ~SCRBundleExtension();
private:
  FRIEND_TEST(SCRBundleExtensionTest, CtorWithValidArgs);

  cppmicroservices::BundleContext bundleContext;
  std::shared_ptr<ComponentRegistry> registry;
  std::shared_ptr<LogService> logger;
  std::vector<std::shared_ptr<ComponentManager>> managers;
};
} // scrimpl
} // cppmicroservices

#endif // __SCRBUNDLEEXTENSION_HPP__
