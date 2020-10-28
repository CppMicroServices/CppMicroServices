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

#ifndef cppmicroservices_service_cm_ConfigurationListeneImpl_HPP
#define cppmicroservices_service_cm_ConfigurationListeneImpl_HPP

#include "ComponentRegistry.hpp"
#include "SCRLogger.hpp"
#include "cppmicroservices/cm/ConfigurationListener.hpp"

namespace cppmicroservices {
namespace service {
namespace cm {

class ConfigurationListenerImpl : public ConfigurationListener
{

public:
  ConfigurationListenerImpl(
    cppmicroservices::BundleContext context,
    std::shared_ptr<cppmicroservices::scrimpl::ComponentRegistry>
      componentRegistry,
    std::shared_ptr<cppmicroservices::logservice::LogService> logger);

  void configurationEvent(const ConfigurationEvent& event) override;

private:
  cppmicroservices::BundleContext bundleContext;
  std::shared_ptr<cppmicroservices::scrimpl::ComponentRegistry> registry;
  std::shared_ptr<cppmicroservices::logservice::LogService> logger;
};

} // namespace cm
} // namespace service
} // namespace cppmicroservices
#endif //cppmicroservices_service_cm_ConfigurationListeneImpl_HPP