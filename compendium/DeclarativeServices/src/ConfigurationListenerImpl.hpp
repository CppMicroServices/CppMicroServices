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
#include "manager/ConcurrencyUtil.hpp"

namespace cppmicroservices {
namespace service {
namespace cm {

  /** ConfigChangeNotification
     * This class is used by ConfigurationListener to notify ComponentConfigurationImpl
     * about changes to Configuration Objects.
     */
struct ConfigChangeNotification final
{
  ConfigChangeNotification(const std::string _pid,
                           std::shared_ptr<cppmicroservices::AnyMap> _properties,
                           const ConfigurationEventType _evt)
    : pid(std::move(_pid))
    , newProperties(_properties)
    , event(std::move(_evt))
  {}

  std::string pid;
  ConfigurationEventType event;
  std::shared_ptr<cppmicroservices::AnyMap> newProperties;
};
class ConfigurationListenerImpl final : public ConfigurationListener
{

public:
  ConfigurationListenerImpl(
    cppmicroservices::BundleContext _context,
    std::shared_ptr<const cppmicroservices::scrimpl::ComponentRegistry>
      _componentRegistry,
    std::shared_ptr<cppmicroservices::logservice::LogService> _logger);
  ConfigurationListenerImpl(const ConfigurationListenerImpl&) = delete;
  ConfigurationListenerImpl(ConfigurationListenerImpl&&) = delete;
  ConfigurationListenerImpl& operator=(const ConfigurationListenerImpl&) =    delete;
  ConfigurationListenerImpl& operator=(ConfigurationListenerImpl&&) = delete;
  ~ConfigurationListenerImpl()  = default;
  
  void configurationEvent(const ConfigurationEvent& event) override;

  cppmicroservices::ListenerTokenId RegisterListener(
    const std::string& pid,
    std::function<void(const ConfigChangeNotification&)> notify);


  void UnregisterListener(const std::string& pid,
    const cppmicroservices::ListenerTokenId token);

 private:
  using TokenMap =
    std::unordered_map<ListenerTokenId,
                       std::function<void(const ConfigChangeNotification&)>>;

   static cppmicroservices::scrimpl::Guarded < std::unordered_map < std::string,
     std::shared_ptr <TokenMap>>> listenersMap;

  static std::atomic<cppmicroservices::ListenerTokenId>
    tokenCounter; ///< used to
                  ///generate unique
                  ///tokens for
                  ///listeners

  cppmicroservices::BundleContext bundleContext;
  const std::shared_ptr<const cppmicroservices::scrimpl::ComponentRegistry> registry;
  std::shared_ptr<cppmicroservices::logservice::LogService> logger;
};

} // namespace cm
} // namespace service
} // namespace cppmicroservices
#endif //cppmicroservices_service_cm_ConfigurationListeneImpl_HPP