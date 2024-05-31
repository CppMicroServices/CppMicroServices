#ifndef CPPMICROSERVICES_LOGGER_H_
#define CPPMICROSERVICES_LOGGER_H_

#include "cppmicroservices/ServiceReferenceBase.h"

#include <cstdint>
#include <exception>
#include <string>
#include <functional>

namespace sinks
{
    class sink;
}

namespace spdlog
{
    class logger;
    using sink_ptr = std::shared_ptr<sinks::sink>;
} // namespace spdlog

namespace cppmicroservices::logservice
{
        /** @}*/

        /**
         * \ingroup MicroService
         * \ingroup gr_logservice
         *
         * Provides methods for bundles to write messages to the log.
         * Logger interface defines several methods for each of the defined LogLevels.
         *
         */

	enum class LogLevel
        {
            audit, //This log level is used for information that must always be logged.
 
            error, //This log level is used for information about an error situation.
 
            warn, //This log level is used for information about a failure or unwanted situation that is not blocking.
 
            info, //This log level is used for information about normal operation.
 
            debug, //This log level is used for detailed output for debugging operations.
 
            trace //This log level is used for large volume of output for tracing operations.   
	};
	
        class Logger
        {
          public:

            virtual ~Logger() = default;

	    Logger() = default;
	     // Copy constructor
    Logger(const Logger& other) = default;

    // Copy assignment operator
    Logger& operator=(const Logger& other) = default;

    // Move constructor
    Logger(Logger&& other) noexcept = default;

    // Move assignment operator
    Logger& operator=(Logger&& other) noexcept = default;

	    virtual void audit(const std::string& message) = 0;
	    virtual void audit(const std::string& format, const std::string& arg) = 0;
	    virtual void audit(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;
	    virtual void audit(std::string const& message, const std::exception_ptr ex) = 0;
            virtual void audit(std::string const& message, ServiceReferenceBase const& sr) = 0;
            virtual void audit(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex) = 0;

	    virtual void debug(const std::string& message) = 0;
            virtual void debug(const std::string& format, const std::string& arg) = 0;
	    virtual void debug(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;
	    virtual void debug(std::string const& message, const std::exception_ptr ex) = 0;
	    virtual void debug(std::string const& message, ServiceReferenceBase const& sr) = 0;
	    virtual void debug(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex) = 0;

	    virtual void error(const std::string& message) = 0;
            virtual void error(const std::string& format, const std::string& arg) = 0;
            virtual void error(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;
	    virtual void error(std::string const& message, const std::exception_ptr ex) = 0;
            virtual void error(std::string const& message, ServiceReferenceBase const& sr) = 0;
            virtual void error(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex) = 0;

	    virtual void info(const std::string& message) = 0;
            virtual void info(const std::string& format, const std::string& arg) = 0;
            virtual void info(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;
	    virtual void info(std::string const& message, const std::exception_ptr ex) = 0;
            virtual void info(std::string const& message, ServiceReferenceBase const& sr) = 0;
            virtual void info(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex) = 0;

	    virtual void trace(const std::string& message) = 0;
            virtual void trace(const std::string& format, const std::string& arg) = 0;
            virtual void trace(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;
	    virtual void trace(std::string const& message, const std::exception_ptr ex) = 0;
            virtual void trace(std::string const& message, ServiceReferenceBase const& sr) = 0;
            virtual void trace(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex) = 0;

	    virtual void warn(const std::string& message) = 0;
            virtual void warn(const std::string& format, const std::string& arg) = 0;
            virtual void warn(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;
	    virtual void warn(std::string const& message, const std::exception_ptr ex) = 0;
            virtual void warn(std::string const& message, ServiceReferenceBase const& sr) = 0;
            virtual void warn(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex) = 0;

	     /**
             * Registers a sink to the logger for introspection of contents. This is not a publicly available
             * function and should only be used for testing. This is NOT thread-safe.
             */
	    virtual void AddSink(spdlog::sink_ptr& sink) = 0;

	};           
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_LOG_SERVICE_H__
