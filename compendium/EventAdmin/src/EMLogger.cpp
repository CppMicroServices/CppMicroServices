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

#include "EMLogger.hpp"

namespace cppmicroservices::emimpl
{

    EMLogger::EMLogger(cppmicroservices::BundleContext context)
        : emContext(context)
        , serviceTracker(
              std::make_unique<cppmicroservices::ServiceTracker<cppmicroservices::logservice::LogService>>(context,
                                                                                                           this))
        , logService(nullptr)
    {
        serviceTracker->Open(); // Start tracking
    }

    EMLogger::~EMLogger() { logService.reset(); }

    void
    EMLogger::StopTracking()
    {
        if (serviceTracker)
        {
            serviceTracker->Close();
            serviceTracker.reset();
        }
    }

    std::shared_ptr<cppmicroservices::logservice::LogService>
    EMLogger::AddingService(ServiceReference<cppmicroservices::logservice::LogService> const& reference)
    {
        auto currLogger = std::atomic_load(&logService);
        std::shared_ptr<cppmicroservices::logservice::LogService> logger;
        if (!currLogger && reference)
        {
            logger = emContext.GetService<cppmicroservices::logservice::LogService>(reference);
            std::atomic_store(&logService, logger);
        }
        return logger;
    }

    void
    EMLogger::ModifiedService(ServiceReference<cppmicroservices::logservice::LogService> const& /*reference*/,
                              std::shared_ptr<cppmicroservices::logservice::LogService> const& /*service*/)
    {
        // no-op. Don't care if properties change
    }

    void
    EMLogger::RemovedService(ServiceReference<cppmicroservices::logservice::LogService> const& /*reference*/,
                             std::shared_ptr<cppmicroservices::logservice::LogService> const& service)
    {
        auto currLogger = std::atomic_load(&logService);
        if (service == currLogger)
        {
            // replace existing logger with a nullptr logger
            std::shared_ptr<cppmicroservices::logservice::LogService> logger(nullptr);
            std::atomic_store(&logService, logger);
        }
    }

    void
    EMLogger::Log(logservice::SeverityLevel level, std::string const& message)
    {
        auto currLogger = std::atomic_load(&logService);
        if (currLogger)
        {
            currLogger->Log(level, message);
        }
    }

    void
    EMLogger::Log(logservice::SeverityLevel level, std::string const& message, std::exception_ptr const ex)
    {
        auto currLogger = std::atomic_load(&logService);
        if (currLogger)
        {
            currLogger->Log(level, message, ex);
        }
    }

    void
    EMLogger::Log(cppmicroservices::ServiceReferenceBase const& sr,
                  logservice::SeverityLevel level,
                  std::string const& message)
    {
        auto currLogger = std::atomic_load(&logService);
        if (currLogger)
        {
            currLogger->Log(sr, level, message);
        }
    }

    void
    EMLogger::Log(cppmicroservices::ServiceReferenceBase const& sr,
                  logservice::SeverityLevel level,
                  std::string const& message,
                  std::exception_ptr const ex)
    {
        auto currLogger = std::atomic_load(&logService);
        if (currLogger)
        {
            currLogger->Log(sr, level, message, ex);
        }
    }
} // namespace cppmicroservices::emimpl
