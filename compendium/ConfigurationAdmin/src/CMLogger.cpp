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

#include "CMLogger.hpp"

namespace cppmicroservices {
  namespace cmimpl {
    CMLogger::CMLogger(cppmicroservices::BundleContext context)
    : cmContext(std::move(context))
    , serviceTracker(std::make_unique<cppmicroservices::ServiceTracker<cppmicroservices::logservice::LogService>>(cmContext, this))
    , logService(nullptr)
    {
      serviceTracker->Open(); // Start tracking
    }

    CMLogger::~CMLogger()
    {
      if (serviceTracker)
      {
        serviceTracker->Close();
      }
    }

    std::shared_ptr<cppmicroservices::logservice::LogService> CMLogger::AddingService(const ServiceReference<cppmicroservices::logservice::LogService>& reference)
    {
      auto currLogger = std::atomic_load(&logService);
      std::shared_ptr<cppmicroservices::logservice::LogService> logger;
      if(!currLogger && reference)
      {
        logger = cmContext.GetService<cppmicroservices::logservice::LogService>(reference);
        std::atomic_store(&logService, logger);
      }
      return logger;
    }

    void CMLogger::ModifiedService(const ServiceReference<cppmicroservices::logservice::LogService>& /*reference*/,
                                    const std::shared_ptr<cppmicroservices::logservice::LogService>& /*service*/)
    {
      // no-op. Don't care if properties change
    }

    void CMLogger::RemovedService(const ServiceReference<cppmicroservices::logservice::LogService>& /*reference*/,
                                   const std::shared_ptr<cppmicroservices::logservice::LogService>& service)
    {
      auto currLogger = std::atomic_load(&logService);
      if(service == currLogger)
      {
        // replace existing logger with a nullptr logger
        std::shared_ptr<cppmicroservices::logservice::LogService> logger(nullptr);
        std::atomic_store(&logService, logger);
      }
    }

    void CMLogger::Log(logservice::SeverityLevel level, const std::string &message)
    {
      auto currLogger = std::atomic_load(&logService);
      if (currLogger)
      {
        currLogger->Log(level, message);
      }
    }

    void CMLogger::Log(logservice::SeverityLevel level,
                        const std::string &message,
                        const std::exception_ptr ex)
    {
      auto currLogger = std::atomic_load(&logService);
      if (currLogger)
      {
        currLogger->Log(level, message, ex);
      }
    }

    void CMLogger::Log(const cppmicroservices::ServiceReferenceBase &sr,
                        logservice::SeverityLevel level,
                        const std::string &message)
    {
      auto currLogger = std::atomic_load(&logService);
      if (currLogger)
      {
        currLogger->Log(sr, level, message);
      }
    }

    void CMLogger::Log(const cppmicroservices::ServiceReferenceBase &sr,
                        logservice::SeverityLevel level,
                        const std::string &message,
                        const std::exception_ptr ex)
    {
      auto currLogger = std::atomic_load(&logService);
      if (currLogger)
      {
        currLogger->Log(sr, level, message, ex);
      }
    }
  } // cmimpl
} // cppmicroservices

