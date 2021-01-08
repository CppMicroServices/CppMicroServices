#include "ConfigurationListenerImpl.hpp"
#include "cppmicroservices/cm/ConfigurationAdmin.hpp"

namespace cppmicroservices {
namespace service {
namespace cm {

std::atomic<cppmicroservices::ListenerTokenId>
  ConfigurationListenerImpl::tokenCounter(0);

cppmicroservices::scrimpl::Guarded<
  std::unordered_map<std::string,std::shared_ptr<ConfigurationListenerImpl::TokenMap>>>
  ConfigurationListenerImpl::listenersMap;

ConfigurationListenerImpl::ConfigurationListenerImpl(
  cppmicroservices::BundleContext _context,
  std::shared_ptr<const cppmicroservices::scrimpl::ComponentRegistry> _componentRegistry,
  std::shared_ptr<cppmicroservices::logservice::LogService> _logger)
  : bundleContext(std::move(_context))
  , registry(std::move(_componentRegistry))
  , logger(std::move(_logger))
{
  if (!bundleContext || !registry || !(this->logger)) {
    throw std::invalid_argument("ConfigurationListenerImpl Constructor "
                                "provided with invalid arguments");
  }
}

void ConfigurationListenerImpl::configurationEvent(const ConfigurationEvent& event)
{
  
  {
    if (listenersMap.lock()->empty())
      return;
  }
  
  auto configAdminRef = event.getReference();
  if (!configAdminRef) 
    return;
  auto configAdmin =
    this->bundleContext
      .GetService<cppmicroservices::service::cm::ConfigurationAdmin>(configAdminRef);

  auto properties =
    cppmicroservices::AnyMap(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
  auto type = event.getType();
  auto pid = event.getPid();

  if (!pid.empty()) {
    if (type == ConfigurationEventType::CM_UPDATED) {
      auto configObject = configAdmin->GetConfiguration(pid);
      if (configObject) {
        properties = configObject->GetProperties();
      }
    }
    auto ptr = std::make_shared<cppmicroservices::AnyMap>(properties);
    ConfigChangeNotification notification = ConfigChangeNotification(pid, ptr, type);
 
    std::shared_ptr <TokenMap> listenersMapCopy;
    {
      auto listenersMapHandle = listenersMap.lock();
      auto iter = listenersMapHandle->find(pid);
      if (iter != listenersMapHandle->end()) {
        listenersMapCopy = iter->second;
      } else {
        return;
      }
    }

     for (auto& configListenerPtr : *listenersMapCopy ) {
      configListenerPtr.second(notification);     
    }

  }


}


cppmicroservices::ListenerTokenId ConfigurationListenerImpl::RegisterListener(const std::string& pid,
  std::function<void(const ConfigChangeNotification&)> notify)
{ 
  cppmicroservices::ListenerTokenId retToken = ++tokenCounter;
  
  {
    auto listenersMapHandle = listenersMap.lock();
    auto iter = listenersMapHandle->find(pid);
    if (iter != listenersMapHandle->end()) {
      (*(iter->second)).emplace(retToken, notify);
    } else {     
       auto tokenMapPtr = std::make_shared<TokenMap>(); 
       tokenMapPtr->emplace(retToken, notify);
       listenersMapHandle->emplace(pid, tokenMapPtr);        
     }  
  }
  
  return retToken;
}


void ConfigurationListenerImpl::UnregisterListener(const std::string& pid
                          , const cppmicroservices::ListenerTokenId token)
{
  
  auto listenersMapHandle = listenersMap.lock();
  if (listenersMapHandle->empty() || pid.empty())
      return;
  
  auto iter = listenersMapHandle->find(pid);
  if (iter == listenersMapHandle->end())
    return;
 
  auto tokenMapPtr = iter->second;
  for (auto& tokenMap : (*tokenMapPtr)) {
    if (tokenMap.first == token) {
        tokenMapPtr->erase(tokenMap.first);
      if (tokenMapPtr->size() == 0){
        listenersMapHandle->erase(iter);
       }
        break;
    }
  }
  
}

} // namespace cm
} // namespace service
} // namespace cppmicroservices
