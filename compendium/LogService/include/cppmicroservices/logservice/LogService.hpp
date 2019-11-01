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
#ifndef CPPMICROSERVICES_LOG_SERVICE_H__
#define CPPMICROSERVICES_LOG_SERVICE_H__

#include "cppmicroservices/logservice/LogServiceExport.h"

#include "cppmicroservices/ServiceReferenceBase.h"

#include <exception>
#include <string>
#include <cstdint>

namespace cppmicroservices {
namespace logservice {

enum class SeverityLevel : uint8_t
{
  LOG_ERROR = 1,    // Indicates the bundle or service may not be functional. Action should be taken to correct this situation.
  LOG_WARNING = 2,  // Indicates a bundle or service is still functioning but may experience problems in the future because of the warning condition.
  LOG_INFO = 3,     // May be the result of any change in the bundle or service and does not indicate a problem.
  LOG_DEBUG = 4     // Used for problem determination and may be irrelevant to anyone but the bundle developer.
};

/**
 * Provides methods for bundles to write messages to the log. 
 * LogService methods are provided to log messages; optionally with a ServiceReference object or an exception.
 * Bundles must log messages in the OSGi environment with a severity level according to the following
 * hierarchy:
 *  1. LOG_ERROR
 *  2. LOG_WARNING
 *  3. LOG_INFO
 *  4. LOG_DEBUG
 *
 * @remarks This class is thread safe.
 */
class US_LogService_EXPORT LogService
{
  public:
  virtual ~LogService();

  /**
   * Logs a message.
   * @param level The severity of the message. This should be one of the defined log levels but may be any integer that is interpreted in a user defined way.
   * @param message Human readable string describing the condition or empty string.
   */
  virtual void Log(SeverityLevel level, const std::string& message) = 0;

  /**
   * Logs a message.
   * @param level The severity of the message. This should be one of the defined log levels but may be any integer that is interpreted in a user defined way.
   * @param message Human readable string describing the condition or empty string.
   * @param ex The exception that reflects the condition or nullptr.
   */
  virtual void Log(SeverityLevel level, const std::string& message, const std::exception_ptr ex) = 0;

  /**
   * Logs a message.
   * @param sr The ServiceReferenceBase object of the service that this message is associated with or an invalid object.
   * @param level The severity of the message. This should be one of the defined log levels but may be any integer that is interpreted in a user defined way.
   * @param message Human readable string describing the condition or empty string.
   */
  virtual void Log(const ServiceReferenceBase& sr, SeverityLevel level, const std::string& message) = 0;

  /**
   * Logs a message with an exception associated and a ServiceReference object.
   * @param sr The ServiceReferenceBase object of the service that this message is associated with or an invalid object.
   * @param level The severity of the message. This should be one of the defined log levels but may be any integer that is interpreted in a user defined way.
   * @param message Human readable string describing the condition or empty string.
   * @param ex The exception that reflects the condition or nullptr.
   */
  virtual void Log(const ServiceReferenceBase& sr, SeverityLevel level, const std::string& message, const std::exception_ptr ex) = 0;
};

} // namespace logservice

} // namespace cppmicroservices

#endif // CPPMICROSERVICES_LOG_SERVICE_H__
