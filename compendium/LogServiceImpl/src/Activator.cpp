#include "Activator.hpp"
#include "LogServiceFactory.hpp"

#include "cppmicroservices/logservice/LogService.hpp"
#include "cppmicroservices/logservice/LoggerFactory.hpp"

namespace cppmicroservices::logservice::impl
{
    void
    Activator::Start(cppmicroservices::BundleContext bc)
    {
        // Registered as a ServiceFactory so that each bundle obtaining the LogService gets an
        // instance which logs to "LogService.<bsn>", <bsn> being that bundle's symbolic name.
        std::shared_ptr<cppmicroservices::ServiceFactory> factory = std::make_shared<LogServiceFactory>();
        bc.RegisterService<cppmicroservices::logservice::LogService, cppmicroservices::logservice::LoggerFactory>(
            std::move(factory));
    }

    void
    Activator::Stop(cppmicroservices::BundleContext)
    {
    }
} // namespace cppmicroservices::logservice::impl

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(cppmicroservices::logservice::impl::Activator)
