#include "ConfigurationListenerImpl.hpp"
#include "cppmicroservices/cm/ConfigurationAdmin.hpp"

namespace cppmicroservices {
namespace service {
namespace cm {

ConfigurationListenerImpl::ConfigurationListenerImpl(
  const cppmicroservices::BundleContext& context,
  std::shared_ptr<cppmicroservices::logservice::LogService> logger,
  std::shared_ptr<cppmicroservices::scrimpl::ConfigurationNotifier>
    configNotifier)
  : bundleContext(context)
  , logger(std::move(logger))
  , configNotifier(std::move(configNotifier))
{
  if (!bundleContext || !(this->logger) || !(this->configNotifier)) {
    throw std::invalid_argument("ConfigurationListenerImpl Constructor "
                                "provided with invalid arguments");
  }
}

void ConfigurationListenerImpl::configurationEvent(
  const ConfigurationEvent& event) noexcept
{
  try {
    auto pid = (!event.getPid().empty()) ? event.getPid() : event.getFactoryPid();
    if (pid.empty()) {
      return;
    }
    if (!configNotifier->AnyListenersForPid(pid)) {
      return;
    }

    auto configAdminRef = event.getReference();
    if (!configAdminRef) {
      logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                  "configurationEvent error. ConfigurationAdmin service "
                  "reference is no longer valid");
      return;
    }
    auto configAdmin =
      bundleContext
        .GetService<cppmicroservices::service::cm::ConfigurationAdmin>(
          configAdminRef);
    if (!configAdmin) {
      logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                  "configurationEvent error. ConfigurationAdmin service "
                  "reference is no longer valid");
      return;
    }
    auto properties =
      cppmicroservices::AnyMap(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
    auto type = event.getType();

    if (type == ConfigurationEventType::CM_UPDATED) {
      auto configObject = configAdmin->GetConfiguration(pid);
      if (configObject) {
        properties = configObject->GetProperties();
      }
    }
    auto ptr = std::make_shared<cppmicroservices::AnyMap>(properties);
    configNotifier->NotifyAllListeners(pid, type, ptr);
  } catch (...) {
    logger->Log(
      cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
      "Exception while executing ConfigurationListener::configEvent method.",
      std::current_exception());
  }
}
} // namespace cm
} // namespace service
} // namespace cppmicroservices
