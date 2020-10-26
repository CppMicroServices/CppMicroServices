#include "ConfigurationListenerImpl.hpp"
#include "cppmicroservices/cm/ConfigurationAdmin.hpp"

namespace cppmicroservices {
namespace service {
namespace cm {

ConfigurationListenerImpl::ConfigurationListenerImpl(
  cppmicroservices::BundleContext context,
  std::shared_ptr<cppmicroservices::scrimpl::ComponentRegistry> componentRegistry,
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

void ConfigurationListenerImpl::configurationEvent(ConfigurationEvent& event) {
  auto configAdminRef = event.getReference();
  auto type = event.getType();
}


} // namespace cm
} // namespace service
} // namespace cppmicroservices
