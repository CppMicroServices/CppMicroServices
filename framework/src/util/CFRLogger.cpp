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

#include "CFRLogger.h"
#include "cppmicroservices/GetBundleContext.h"

namespace cppmicroservices
{
    namespace cfrimpl
    {
        CFRLogger::CFRLogger() : serviceTracker(), logService(nullptr) { }

	 CFRLogger::CFRLogger(cppmicroservices::BundleContext context)
            : cfrContext(std::move(context))
	      ,serviceTracker(
                  std::make_unique<cppmicroservices::ServiceTracker<cppmicroservices::logservice::LogService>>(cfrContext,
                                                                                                               this))
              ,logService(nullptr) {
               serviceTracker->Open();
    }

        CFRLogger::~CFRLogger()
        {
            try
            {
                this->Close();
            }
            catch (...)
            {
            }
        }

        std::shared_ptr<cppmicroservices::logservice::LogService>
        CFRLogger::AddingService(ServiceReference<cppmicroservices::logservice::LogService> const& reference)
        {
            auto currLogger = std::atomic_load(&logService);
            std::shared_ptr<cppmicroservices::logservice::LogService> logger;
            if (!currLogger && reference)
            {
                logger = cfrContext.GetService<cppmicroservices::logservice::LogService>(reference);
                std::atomic_store(&logService, logger);
            }
            return logger;
        }

        void
        CFRLogger::ModifiedService(ServiceReference<cppmicroservices::logservice::LogService> const& /*reference*/,
                                   std::shared_ptr<cppmicroservices::logservice::LogService> const& /*service*/)
        {
            // no-op. Don't care if properties change
        }

        void
        CFRLogger::RemovedService(ServiceReference<cppmicroservices::logservice::LogService> const& /*reference*/,
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
        CFRLogger::Log(logservice::SeverityLevel level, std::string const& message)
        {
            auto currLogger = std::atomic_load(&logService);
            if (currLogger)
            {
                currLogger->Log(level, message);
            }
        }

        void
        CFRLogger::Log(logservice::SeverityLevel level, std::string const& message, const std::exception_ptr ex)
        {
            auto currLogger = std::atomic_load(&logService);
            if (currLogger)
            {
                currLogger->Log(level, message, ex);
            }
        }

        void
        CFRLogger::Log(cppmicroservices::ServiceReferenceBase const& sr,
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
        CFRLogger::Log(cppmicroservices::ServiceReferenceBase const& sr,
                       logservice::SeverityLevel level,
                       std::string const& message,
                       const std::exception_ptr ex)
        {
            auto currLogger = std::atomic_load(&logService);
            if (currLogger)
            {
                currLogger->Log(sr, level, message, ex);
            }
        }

	std::shared_ptr<cppmicroservices::logservice::Logger>
        CFRLogger::getLogger(const std::string& name) const
	{
	     auto currLogger = std::atomic_load(&logService);
	     if (currLogger)
	     {
	     	return currLogger->getLogger(name);
	     }
	     return nullptr;
	}

	std::shared_ptr<cppmicroservices::logservice::Logger>
        CFRLogger::getLogger(const cppmicroservices::Bundle& bundle, const std::string& name) const
        {
             auto currLogger = std::atomic_load(&logService);
	     if(currLogger)
	     {
		return currLogger->getLogger(bundle, name);
	     }
	     return nullptr;
        }

        void
        CFRLogger::Close()
        {
            auto l = this->Lock();
            US_UNUSED(l);
            if (serviceTracker)
            {
                serviceTracker->Close();
                serviceTracker.reset();
            }
        }
    } // namespace cfrimpl
} // namespace cppmicroservices
