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
  ConfigChangeNotification(const std::string pid,
                           std::shared_ptr<cppmicroservices::AnyMap> properties,
                           const ConfigurationEventType evt)
    : pid(std::move(pid))
    , newProperties(properties)
    , event(std::move(evt))
  {}

  std::string pid;
  ConfigurationEventType event;
  std::shared_ptr<cppmicroservices::AnyMap> newProperties;
};

struct ConfigListenerMapItem final
{
  ConfigListenerMapItem(
    ListenerTokenId token,
    std::function<void(const ConfigChangeNotification&)> notify)
    : token(token)
    , notify(notify)
  {}

  ListenerTokenId token;
  std::function<void(const ConfigChangeNotification&)> notify;
};

class ConfigurationListenerImpl final : public ConfigurationListener
{

public:
  ConfigurationListenerImpl(
    cppmicroservices::BundleContext context,
    std::shared_ptr<const cppmicroservices::scrimpl::ComponentRegistry>
      componentRegistry,
    std::shared_ptr<cppmicroservices::logservice::LogService> logger);

  void configurationEvent(const ConfigurationEvent& event) override;

  cppmicroservices::ListenerTokenId RegisterListener(
    const std::string pid,
    std::function<void(const ConfigChangeNotification&)> notify);

  void UnregisterListener(
    cppmicroservices::ListenerTokenId token);

  //void BatchNotifyAllListeners(
    //const std::vector<ConfigChangeNotification>& notification) noexcept;

 
  //static Guarded<std::multimap<std::string, std::shared_ptr<ConfigListenerMapItem>>> listenersMap; ///< guarded map of listeners
  static std::mutex listenersMapLock;
  static std::multimap<std::string, std::shared_ptr<ConfigListenerMapItem>> listenersMap;

private:
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