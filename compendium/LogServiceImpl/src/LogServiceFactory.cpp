#include "LogServiceFactory.hpp"
#include "LogServiceImpl.hpp"

#include "cppmicroservices/ServiceInterface.h"

namespace cppmicroservices::logservice::impl
{
    std::string
    LogServiceFactory::GetLoggerName(cppmicroservices::Bundle const& bundle)
    { return "LogService." + bundle.GetSymbolicName(); }

    InterfaceMapConstPtr
    LogServiceFactory::GetService(cppmicroservices::Bundle const& bundle,
                                  cppmicroservices::ServiceRegistrationBase const& /*registration*/)
    {
        auto svc = std::make_shared<cppmicroservices::logservice::LogServiceImpl>(GetLoggerName(bundle));
        return MakeInterfaceMap<cppmicroservices::logservice::LogService, cppmicroservices::logservice::LoggerFactory>(
            std::move(svc));
    }

    void
    LogServiceFactory::UngetService(cppmicroservices::Bundle const& /*bundle*/,
                                    cppmicroservices::ServiceRegistrationBase const& /*registration*/,
                                    InterfaceMapConstPtr const& /*service*/)
    {
        // The framework drops its reference to the instance; nothing else to release.
    }
} // namespace cppmicroservices::logservice::impl
