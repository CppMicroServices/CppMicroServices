#ifndef CPPMICROSERVICES_LOGGER_H_
#define CPPMICROSERVICES_LOGGER_H_

#include "cppmicroservices/ServiceReferenceBase.h"

#include <cstdint>
#include <exception>
#include <string>
#include <functional>


namespace cppmicroservices::logservice
{
        /**
        \defgroup gr_logservice Logger

        \brief Groups Logger class related symbols.
        */

	/**
         * \addtogroup gr_logservice
         * @{
        */
	enum class LogLevelCopy
	{
	    Audit, //This log level is used for information that must always be logged. 
            Error, //This log level is used for information about an error situation.
	    Warn,  //This log level is used for information about a failure or unwanted situation that is not blocking.
	    Info,  //This log level is used for information about normal operation.
	    Debug, //This log level is used for detailed output for debugging operations.
	    Trace  //This log level is used for large volume of output for tracing operations. 
	};
	/** @}*/

        /**
         * \ingroup MicroService
         * \ingroup gr_logservice
         *
         * Provides methods for bundles to write messages to the log.
         * Logger interface defines several methods for each of the defined LogLevels.
         *
	 * @remarks This class is thread-safe.
         */

        class Logger
        {
          public:

            virtual ~Logger() = default;
	    
           /**
            * Logs a message at LogLevel.Audit level.
            * @param message The message to log.
            */
	    virtual void audit(const std::string& message) = 0;

	   /**
            * Logs a message at LogLevel.Audit level.
            * @param format The format of the message to log.
            * @param arg The argument to format into the message.
            */
	    virtual void audit(const std::string& format, const std::string& arg) = 0;

	   /**
            * Logs a message at LogLevel.Audit level.
            * @param format The format of the message to log.
            * @param arg1 The first argument to format into the message.
	    * @param arg2 The second argument to format into the message.
            */
	    virtual void audit(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;

	   /**
            * Logs a message at LogLevel.Audit level.
            * @param message The message to log.
            * @param ex The exception that reflects the condition or nullptr.
            */
	    virtual void audit(const std::string& message, const std::exception_ptr ex) = 0;

	   /**
            * Logs a message at LogLevel.Audit level.
            * @param message The message to log.
            * @param sr The ServiceReferenceBase object of the service that this message is associated with or an
	    * invalid object.
            */
            virtual void audit(const std::string& message, const ServiceReferenceBase& sr) = 0;

	   /**
            * Logs a message at LogLevel.Audit level.
            * @param message The message to log.
            * @param sr The ServiceReferenceBase object of the service that this message is associated with or an
            * invalid object.
	    * @param ex The exception that reflects the condition or nullptr.
            */
            virtual void audit(const std::string& message, const ServiceReferenceBase& sr, const std::exception_ptr ex) = 0;

	   /**
            * Logs a message at LogLevel.Debug level.
            * @param message The message to log.
            */
	    virtual void debug(const std::string& message) = 0;

	   /**
            * Logs a message at LogLevel.Debug level.
            * @param format The format of the message to log.
            * @param arg The argument to format into the message.
            */
            virtual void debug(const std::string& format, const std::string& arg) = 0;

	   /**
            * Logs a message at LogLevel.Debug level.
            * @param format The format of the message to log.
            * @param arg1 The first argument to format into the message.
            * @param arg2 The second argument to format into the message.
            */
	    virtual void debug(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;

	   /**
            * Logs a message at LogLevel.Debug level.
            * @param message The message to log.
            * @param ex The exception that reflects the condition or nullptr.
            */
	    virtual void debug(const std::string& message, const std::exception_ptr ex) = 0;

	   /**
            * Logs a message at LogLevel.Debug level.
            * @param message The message to log.
            * @param sr The ServiceReferenceBase object of the service that this message is associated with or an
            * invalid object.
            */
	    virtual void debug(const std::string& message, const ServiceReferenceBase& sr) = 0;

	   /**
            * Logs a message at LogLevel.Debug level.
            * @param message The message to log.
            * @param sr The ServiceReferenceBase object of the service that this message is associated with or an
            * invalid object.
            * @param ex The exception that reflects the condition or nullptr.
            */
	    virtual void debug(const std::string& message, const ServiceReferenceBase& sr, const std::exception_ptr ex) = 0;

	   /**
            * Logs a message at LogLevel.Error level.
            * @param message The message to log.
            */
	    virtual void error(const std::string& message) = 0;

	   /**
            * Logs a message at LogLevel.Error level.
            * @param format The format of the message to log.
            * @param arg The argument to format into the message.
            */
            virtual void error(const std::string& format, const std::string& arg) = 0;

	   /**
            * Logs a message at LogLevel.Error level.
            * @param format The format of the message to log.
            * @param arg1 The first argument to format into the message.
            * @param arg2 The second argument to format into the message.
            */
            virtual void error(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;

	   /**
            * Logs a message at LogLevel.Error level.
            * @param message The message to log.
            * @param ex The exception that reflects the condition or nullptr.
            */
	    virtual void error(const std::string& message, const std::exception_ptr ex) = 0;

	   /**
            * Logs a message at LogLevel.Error level.
            * @param message The message to log.
            * @param sr The ServiceReferenceBase object of the service that this message is associated with or an
            * invalid object.
            */
            virtual void error(const std::string& message, const ServiceReferenceBase& sr) = 0;

	   /**
            * Logs a message at LogLevel.Error level.
            * @param message The message to log.
            * @param sr The ServiceReferenceBase object of the service that this message is associated with or an
            * invalid object.
            * @param ex The exception that reflects the condition or nullptr.
            */
            virtual void error(const std::string& message, const ServiceReferenceBase& sr, const std::exception_ptr ex) = 0;

	   /**
            * Logs a message at LogLevel.Info level.
            * @param message The message to log.
            */
	    virtual void info(const std::string& message) = 0;

	   /**
            * Logs a message at LogLevel.Info level.
            * @param format The format of the message to log.
            * @param arg The argument to format into the message.
            */
            virtual void info(const std::string& format, const std::string& arg) = 0;

	   /**
            * Logs a message at LogLevel.Info level.
            * @param format The format of the message to log.
            * @param arg1 The first argument to format into the message.
            * @param arg2 The second argument to format into the message.
            */
            virtual void info(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;

	   /**
            * Logs a message at LogLevel.Info level.
            * @param message The message to log.
            * @param ex The exception that reflects the condition or nullptr.
            */
	    virtual void info(const std::string& message, const std::exception_ptr ex) = 0;

	   /**
            * Logs a message at LogLevel.Info level.
            * @param message The message to log.
            * @param sr The ServiceReferenceBase object of the service that this message is associated with or an
            * invalid object.
            */
            virtual void info(const std::string& message, const ServiceReferenceBase& sr) = 0;

	   /**
            * Logs a message at LogLevel.Info level.
            * @param message The message to log.
            * @param sr The ServiceReferenceBase object of the service that this message is associated with or an
            * invalid object.
            * @param ex The exception that reflects the condition or nullptr.
            */
            virtual void info(const std::string& message, const ServiceReferenceBase& sr, const std::exception_ptr ex) = 0;

	   /**
            * Logs a message at LogLevel.Trace level.
            * @param message The message to log.
            */
	    virtual void trace(const std::string& message) = 0;

	   /**
            * Logs a message at LogLevel.Trace level.
            * @param format The format of the message to log.
            * @param arg The argument to format into the message.
            */
            virtual void trace(const std::string& format, const std::string& arg) = 0;

	   /**
            * Logs a message at LogLevel.Trace level.
            * @param format The format of the message to log.
            * @param arg1 The first argument to format into the message.
            * @param arg2 The second argument to format into the message.
            */
            virtual void trace(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;

	   /**
            * Logs a message at LogLevel.Trace level.
            * @param message The message to log.
            * @param ex The exception that reflects the condition or nullptr.
            */
	    virtual void trace(const std::string& message, const std::exception_ptr ex) = 0;

	   /**
            * Logs a message at LogLevel.Trace level.
            * @param message The message to log.
            * @param sr The ServiceReferenceBase object of the service that this message is associated with or an
            * invalid object.
            */
            virtual void trace(const std::string& message, const ServiceReferenceBase& sr) = 0;

	   /**
            * Logs a message at LogLevel.Trace level.
            * @param message The message to log.
            * @param sr The ServiceReferenceBase object of the service that this message is associated with or an
            * invalid object.
            * @param ex The exception that reflects the condition or nullptr.
            */
            virtual void trace(const std::string& message, const ServiceReferenceBase& sr, const std::exception_ptr ex) = 0;

	   /**
            * Logs a message at LogLevel.Warn level.
            * @param message The message to log.
            */
	    virtual void warn(const std::string& message) = 0;

	   /**
            * Logs a message at LogLevel.Warn level.
            * @param format The format of the message to log.
            * @param arg The argument to format into the message.
            */
            virtual void warn(const std::string& format, const std::string& arg) = 0;

	   /**
            * Logs a message at LogLevel.Warn level.
            * @param format The format of the message to log.
            * @param arg1 The first argument to format into the message.
            * @param arg2 The second argument to format into the message.
            */
            virtual void warn(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;

	   /**
            * Logs a message at LogLevel.Warn level.
            * @param message The message to log.
            * @param ex The exception that reflects the condition or nullptr.
            */
	    virtual void warn(const std::string& message, const std::exception_ptr ex) = 0;

	   /**
            * Logs a message at LogLevel.Warn level.
            * @param message The message to log.
            * @param sr The ServiceReferenceBase object of the service that this message is associated with or an
            * invalid object.
            */
            virtual void warn(const std::string& message, const ServiceReferenceBase& sr) = 0;

	   /**
            * Logs a message at LogLevel.Warn level.
            * @param message The message to log.
            * @param sr The ServiceReferenceBase object of the service that this message is associated with or an
            * invalid object.
            * @param ex The exception that reflects the condition or nullptr.
            */
            virtual void warn(const std::string& message, const ServiceReferenceBase& sr, const std::exception_ptr ex) = 0;
	    
	};           
} // namespace cppmicroservices::logservice

#endif // CPPMICROSERVICES_LOG_SERVICE_H__
