#include "ConfigurationNotifier.hpp"
#include "cppmicroservices/cm/ConfigurationAdmin.hpp"

namespace cppmicroservices {
namespace scrimpl {

ConfigurationNotifier::ConfigurationNotifier(
   cppmicroservices::BundleContext context,
   std::shared_ptr<cppmicroservices::logservice::LogService> logger)
  : bundleContext(std::move(context))
  , logger(std::move(logger)) 
  , tokenCounter(0)
{
  if (!bundleContext || !(this->logger)) {
    throw std::invalid_argument("ConfigurationNotifier Constructor "
                                "provided with invalid arguments");
  }
}


cppmicroservices::ListenerTokenId ConfigurationNotifier::RegisterListener(
  const std::string& pid,
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

void ConfigurationNotifier::UnregisterListener(
  const std::string& pid,
  const cppmicroservices::ListenerTokenId token) noexcept
{
  auto listenersMapHandle = listenersMap.lock();
  if (listenersMapHandle->empty() || pid.empty()) {
    return;
  }

  auto iter = listenersMapHandle->find(pid);
  if (iter == listenersMapHandle->end()) {
    return;
  }

  auto tokenMapPtr = iter->second;
  for (const auto& tokenMap : (*tokenMapPtr)) {
    if (tokenMap.first == token) {
      tokenMapPtr->erase(tokenMap.first);
      if (tokenMapPtr->size() == 0) {
        listenersMapHandle->erase(iter);
      }
      break;
    }
  }
}
bool ConfigurationNotifier::AnyListenersForPid(const std::string& pid) noexcept
{
  auto listenersMapHandle = listenersMap.lock();
  if (listenersMapHandle->empty() || pid.empty()){
    return false;
  }

   auto iter = listenersMapHandle->find(pid);
   return iter != listenersMapHandle->end();  
}

void ConfigurationNotifier::NotifyAllListeners(
  const std::string& pid,
  const cppmicroservices::service::cm::ConfigurationEventType type,
  const std::shared_ptr<cppmicroservices::AnyMap> properties )
  {
     ConfigChangeNotification notification =
      ConfigChangeNotification(pid, properties, type);
   
    std::shared_ptr<TokenMap> listenersMapCopy;
    {
      auto listenersMapHandle = listenersMap.lock();
      auto iter = listenersMapHandle->find(pid);
      if (iter != listenersMapHandle->end()) {
        listenersMapCopy = iter->second;
      } else {
        return;
      }
    }
    for (const auto& configListenerPtr : *listenersMapCopy) {
      configListenerPtr.second(notification);
    }
  }

} // namespace scrimpl
} // namespace cppmicroservices
