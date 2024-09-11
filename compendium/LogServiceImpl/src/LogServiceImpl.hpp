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

#include "cppmicroservices/logservice/LogService.hpp"
#include "cppmicroservices/logservice/Logger.hpp"
#include "cppmicroservices/logservice/LoggerFactory.hpp"
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Bundle.h>

namespace sinks
{
    class sink;
}

namespace spdlog
{
    class logger;
    using sink_ptr = std::shared_ptr<sinks::sink>;
} // namespace spdlog

using namespace cppmicroservices;

namespace cppmicroservices
{
    namespace logservice
    {
        class LogServiceImpl final : public LogService
        {
          public:
            LogServiceImpl(std::string const& loggerName);
            ~LogServiceImpl() = default;

            /**
             * Logs a message.
             * @param level The severity of the message. This should be one of the defined log levels but may be any
             * integer that is interpreted in a user defined way.
             * @param message Human readable string describing the condition or empty string.
             */
            void Log(SeverityLevel level, std::string const& message) override;

            /**
             * Logs a message.
             * @param level The severity of the message. This should be one of the defined log levels but may be any
             * integer that is interpreted in a user defined way.
             * @param message Human readable string describing the condition or empty string.
             * @param ex The exception that reflects the condition or nullptr.
             */
            void Log(SeverityLevel level, std::string const& message, const std::exception_ptr ex) override;

            /**
             * Logs a message.
             * @param sr The ServiceReferenceBase object of the service that this message is associated with or an
             * invalid object.
             * @param level The severity of the message. This should be one of the defined log levels but may be any
             * integer that is interpreted in a user defined way.
             * @param message Human readable string describing the condition or empty string.
             */
            void Log(ServiceReferenceBase const& sr, SeverityLevel level, std::string const& message) override;

            /**
             * Logs a message with an exception associated and a ServiceReference object.
             * @param sr The ServiceReferenceBase object of the service that this message is associated with or an
             * invalid object.
             * @param level The severity of the message. This should be one of the defined log levels but may be any
             * integer that is interpreted in a user defined way.
             * @param message Human readable string describing the condition or empty string.
             * @param ex The exception that reflects the condition or nullptr.
             */
            void Log(ServiceReferenceBase const& sr,
                     SeverityLevel level,
                     std::string const& message,
                     const std::exception_ptr ex) override;

            [[nodiscard]] std::shared_ptr<Logger> getLogger(std::string const& name) const override;
            [[nodiscard]] std::shared_ptr<Logger> getLogger(const cppmicroservices::Bundle& bundle, std::string const& name) const override;

	    template <typename... Args>
            void logImpl(SeverityLevel level, Args&&... args)
            {
                auto currLogger = std::atomic_load(&logger);
                if (!currLogger)
                {
                    return;
                }

                switch (level)
                {
                    case SeverityLevel::LOG_DEBUG:
                        currLogger->debug(std::forward<Args>(args)...);
                        break;
                    case SeverityLevel::LOG_INFO:
                        currLogger->info(std::forward<Args>(args)...);
                        break;
                    case SeverityLevel::LOG_WARNING:
                        currLogger->warn(std::forward<Args>(args)...);
                        break;
                    case SeverityLevel::LOG_ERROR:
                        currLogger->error(std::forward<Args>(args)...);
                        break;
                    default:
                        currLogger->trace(std::forward<Args>(args)...);
                        break;
                }
            }

            /**
             * Registers a sink to the logger for introspection of contents. This is not a publicly available
             * function and should only be used for testing. This is NOT thread-safe.
             */
            void AddSink(spdlog::sink_ptr& sink);

          private:
             std::shared_ptr<Logger> logger;
        };
    } // namespace logservice
} // namespace cppmicroservices
