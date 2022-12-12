#include "Activator.hpp"
#include "LogServiceImpl.hpp"

namespace cppmicroservices
{
    namespace logservice
    {
        namespace impl
        {
            void
            Activator::Start(cppmicroservices::BundleContext bc)
            {
                auto svc
                    = std::make_shared<cppmicroservices::logservice::LogServiceImpl>("cppmicroservices::logservice");
                bc.RegisterService<cppmicroservices::logservice::LogService>(std::move(svc));
            }

            void
            Activator::Stop(cppmicroservices::BundleContext)
            {
            }
        } // namespace impl
    }     // namespace logservice
} // namespace cppmicroservices

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(cppmicroservices::logservice::impl::Activator)
