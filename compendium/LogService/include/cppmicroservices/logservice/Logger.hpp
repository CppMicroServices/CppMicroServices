#ifndef CPPMICROSERVICES_LOGGER_H_
#define CPPMICROSERVICES_LOGGER_H_

#include "cppmicroservices/ServiceReferenceBase.h"

#include <cstdint>
#include <exception>
#include <string>
#include <functional>


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

        class Logger
        {
          public:

            virtual ~Logger() = default;

	    virtual void audit(const std::string& message) = 0;
	    virtual void audit(const std::string& format, const std::string& arg) = 0;
	    virtual void audit(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;
	    virtual void audit(const std::string& message, const std::exception_ptr ex) = 0;
            virtual void audit(const std::string& message, const ServiceReferenceBase& sr) = 0;
            virtual void audit(const std::string& message, const ServiceReferenceBase& sr, const std::exception_ptr ex) = 0;

	    virtual void debug(const std::string& message) = 0;
            virtual void debug(const std::string& format, const std::string& arg) = 0;
	    virtual void debug(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;
	    virtual void debug(const std::string& message, const std::exception_ptr ex) = 0;
	    virtual void debug(const std::string& message, const ServiceReferenceBase& sr) = 0;
	    virtual void debug(const std::string& message, const ServiceReferenceBase& sr, const std::exception_ptr ex) = 0;

	    virtual void error(const std::string& message) = 0;
            virtual void error(const std::string& format, const std::string& arg) = 0;
            virtual void error(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;
	    virtual void error(const std::string& message, const std::exception_ptr ex) = 0;
            virtual void error(const std::string& message, const ServiceReferenceBase& sr) = 0;
            virtual void error(const std::string& message, const ServiceReferenceBase& sr, const std::exception_ptr ex) = 0;

	    virtual void info(const std::string& message) = 0;
            virtual void info(const std::string& format, const std::string& arg) = 0;
            virtual void info(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;
	    virtual void info(const std::string& message, const std::exception_ptr ex) = 0;
            virtual void info(const std::string& message, const ServiceReferenceBase& sr) = 0;
            virtual void info(const std::string& message, const ServiceReferenceBase& sr, const std::exception_ptr ex) = 0;

	    virtual void trace(const std::string& message) = 0;
            virtual void trace(const std::string& format, const std::string& arg) = 0;
            virtual void trace(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;
	    virtual void trace(const std::string& message, const std::exception_ptr ex) = 0;
            virtual void trace(const std::string& message, const ServiceReferenceBase& sr) = 0;
            virtual void trace(const std::string& message, const ServiceReferenceBase& sr, const std::exception_ptr ex) = 0;

	    virtual void warn(const std::string& message) = 0;
            virtual void warn(const std::string& format, const std::string& arg) = 0;
            virtual void warn(const std::string& format, const std::string& arg1, const std::string& arg2) = 0;
	    virtual void warn(const std::string& message, const std::exception_ptr ex) = 0;
            virtual void warn(const std::string& message, const ServiceReferenceBase& sr) = 0;
            virtual void warn(const std::string& message, const ServiceReferenceBase& sr, const std::exception_ptr ex) = 0;
	    
	};           
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_LOG_SERVICE_H__
