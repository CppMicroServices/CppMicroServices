#include "Activator.hpp"
#include "LoggerFactoryImpl.hpp"
#include "LogServiceImpl.hpp"

namespace cppmicroservices::logservice::impl
{
    void
    Activator::Start(cppmicroservices::BundleContext bc)
    {
        const cppmicroservices::Bundle bundle = bc.GetBundle();
        const std::string bsn = bundle.GetSymbolicName();
        const std::string logger_name = "LogService." + bsn;
        auto svc = std::make_shared<cppmicroservices::logservice::LogServiceImpl>(logger_name);
        bc.RegisterService<cppmicroservices::logservice::LogService, cppmicroservices::logservice::LoggerFactory>(
            std::move(svc));
    }

    void
    Activator::Stop(cppmicroservices::BundleContext)
    {
    }
} // namespace cppmicroservices::logservice::impl

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(cppmicroservices::logservice::impl::Activator)
