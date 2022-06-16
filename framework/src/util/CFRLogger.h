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

#ifndef CFRLOGGER_HPP
#define CFRLOGGER_HPP

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceTracker.h"
#include "cppmicroservices/logservice/LogService.hpp"

namespace cppmicroservices {
namespace cfrimpl {
/**
 * This class is used to track the availability of LogService in the
 * framework. If a LogService is available the calls to Log methods
 * are forwarded to the LogService. Otherwise, the calls to Log methods
 * are no-op calls. This class implements the LogService interface so that
 * other classes within the runtime can easily use a mock log service for
 * testing purposes.
 *
 * Note: This is directly copied from CMLogger.hpp. The only difference
 * is that the name of the class has been changed.
 *
 * TODO(achristoforides): Provide unified "utility" library for the core framework,
 *   DS, CA, and future services containing an implementation of this class, the one
 *   for the AsyncWorkService, and any others that may be needed.
 */
class CFRLogger final
  : public cppmicroservices::logservice::LogService
  , public cppmicroservices::ServiceTrackerCustomizer<
      cppmicroservices::logservice::LogService>
{
public:
  explicit CFRLogger(cppmicroservices::BundleContext context);
  CFRLogger(const CFRLogger&) = delete;
  CFRLogger(CFRLogger&&) = delete;
  CFRLogger& operator=(const CFRLogger&) = delete;
  CFRLogger& operator=(CFRLogger&&) = delete;
  ~CFRLogger() override;

  // methods from the cppmicroservices::logservice::LogService interface
  void Log(logservice::SeverityLevel level,
           const std::string& message) override;
  void Log(logservice::SeverityLevel level,
           const std::string& message,
           const std::exception_ptr ex) override;
  void Log(const ServiceReferenceBase& sr,
           logservice::SeverityLevel level,
           const std::string& message) override;
  void Log(const ServiceReferenceBase& sr,
           logservice::SeverityLevel level,
           const std::string& message,
           const std::exception_ptr ex) override;

  // methods from the cppmicroservices::ServiceTrackerCustomizer interface
  std::shared_ptr<TrackedParamType> AddingService(
    const ServiceReference<cppmicroservices::logservice::LogService>& reference)
    override;
  void ModifiedService(
    const ServiceReference<cppmicroservices::logservice::LogService>& reference,
    const std::shared_ptr<cppmicroservices::logservice::LogService>& service)
    override;
  void RemovedService(
    const ServiceReference<cppmicroservices::logservice::LogService>& reference,
    const std::shared_ptr<cppmicroservices::logservice::LogService>& service)
    override;

private:
  cppmicroservices::BundleContext cfrContext;
  std::unique_ptr<
    cppmicroservices::ServiceTracker<cppmicroservices::logservice::LogService>>
    serviceTracker;
  std::shared_ptr<cppmicroservices::logservice::LogService> logService;
};
} // cfrimpl
} // cppmicroservices

#endif // CFRLOGGER_HPP
