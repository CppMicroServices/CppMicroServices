#include "ConfigurationListenerImpl.hpp"
#include "cppmicroservices/cm/ConfigurationAdmin.hpp"

namespace cppmicroservices {
namespace service {
namespace cm {

ConfigurationListenerImpl::ConfigurationListenerImpl(
  cppmicroservices::BundleContext context,
  std::shared_ptr<const cppmicroservices::scrimpl::ComponentRegistry> componentRegistry,
  std::shared_ptr<cppmicroservices::logservice::LogService> logger)
  : bundleContext(std::move(context))
  , registry(std::move(componentRegistry))
  , logger(std::move(logger))
{
  if (!bundleContext || !registry || !(this->logger)) {
    throw std::invalid_argument("ConfigurationListenerImpl Constructor "
                                "provided with invalid arguments");
  }
}

void ConfigurationListenerImpl::configurationEvent(const ConfigurationEvent& event) {
  auto configAdminRef = event.getReference();
  auto type = event.getType();
}
std::atomic<cppmicroservices::ListenerTokenId>
  ConfigurationListenerImpl::tokenCounter(0);
std::mutex ConfigurationListenerImpl::listenersMapLock;
std::multimap<std::string, std::shared_ptr<ConfigListenerMapItem>>
  ConfigurationListenerImpl::listenersMap;

cppmicroservices::ListenerTokenId ConfigurationListenerImpl::RegisterListener(const std::string pid,
  std::function<void(const ConfigChangeNotification&)> notify)
{
  cppmicroservices::ListenerTokenId retToken = ++tokenCounter;
  
  auto ptr = std::make_shared<ConfigListenerMapItem>(retToken, notify);
  {
    std::lock_guard<std::mutex> guard(listenersMapLock);
    listenersMap.emplace(pid, ptr);
  }
  
  return retToken;
}

void ConfigurationListenerImpl::UnregisterListener(cppmicroservices::ListenerTokenId token)
{
  {
    std::lock_guard<std::mutex> guard(listenersMapLock);
     
    //Too long to keep the lock tied up? More efficient way?
    //Alternative. unordered_map key = pid, value is a vector of ConfigListenerMapItems
    //input here will be pid and token. Would unordered_map make BatchNotifyAllListeners quicker too?
    for (auto iter = listenersMap.begin(); iter != listenersMap.end(); ++iter) {
      auto mapItem = iter->second;
      if (mapItem->token == token) {
        listenersMap.erase(iter);
        break;
      }
    }
    
  }
}

} // namespace cm
} // namespace service
} // namespace cppmicroservices
