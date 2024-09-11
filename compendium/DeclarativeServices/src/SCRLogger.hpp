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

#ifndef SCRLOGGER_HPP
#define SCRLOGGER_HPP
#include <cppmicroservices/ServiceTracker.h>
#include <cppmicroservices/logservice/LogService.hpp>

namespace cppmicroservices
{
    namespace scrimpl
    {
        /**
         * This class is used to track the availability of LogService in the
         * framework. If a LogService is available the calls to Log methods
         * are forwarded to the LogService. Otherwise, the calls to Log methods
         * are no-op calls. This class implements the LogService interface so that
         * other classes within the runtime can easily use a mock log service for
         * testing purposes.
         */
        class SCRLogger
            : public cppmicroservices::logservice::LogService
            , public cppmicroservices::ServiceTrackerCustomizer<cppmicroservices::logservice::LogService>
        {
          public:
            explicit SCRLogger(cppmicroservices::BundleContext context);
            SCRLogger(SCRLogger const&) = delete;
            SCRLogger(SCRLogger&&) = delete;
            SCRLogger& operator=(SCRLogger const&) = delete;
            SCRLogger& operator=(SCRLogger&&) = delete;
            ~SCRLogger() override;

            // methods from the cppmicroservices::logservice::LogService interface
            void Log(logservice::SeverityLevel level, std::string const& message) override;
            void Log(logservice::SeverityLevel level, std::string const& message, const std::exception_ptr ex) override;
            void Log(ServiceReferenceBase const& sr,
                     logservice::SeverityLevel level,
                     std::string const& message) override;
            void Log(ServiceReferenceBase const& sr,
                     logservice::SeverityLevel level,
                     std::string const& message,
                     const std::exception_ptr ex) override;
	    [[nodiscard]] std::shared_ptr<cppmicroservices::logservice::Logger> getLogger(const std::string& name) const override;
	    [[nodiscard]] std::shared_ptr<cppmicroservices::logservice::Logger> getLogger(const cppmicroservices::Bundle& bundle, const std::string& name) const override;

            // methods from the cppmicroservices::ServiceTrackerCustomizer interface
            std::shared_ptr<TrackedParamType> AddingService(
                ServiceReference<cppmicroservices::logservice::LogService> const& reference) override;
            void ModifiedService(ServiceReference<cppmicroservices::logservice::LogService> const& reference,
                                 std::shared_ptr<cppmicroservices::logservice::LogService> const& service) override;
            void RemovedService(ServiceReference<cppmicroservices::logservice::LogService> const& reference,
                                std::shared_ptr<cppmicroservices::logservice::LogService> const& service) override;

            // method to stop tracking the Log Service. This must be called from the SCR BundleActivator's Stop method.
            // Not Thread-safe. must not be called simultaneously from multiple threads
            void StopTracking();

          private:
            cppmicroservices::BundleContext scrContext;
            std::unique_ptr<cppmicroservices::ServiceTracker<cppmicroservices::logservice::LogService>> serviceTracker;
            std::shared_ptr<cppmicroservices::logservice::LogService> logService;
        };
    } // namespace scrimpl
} // namespace cppmicroservices
#endif // SCRLOGGER_HPP
