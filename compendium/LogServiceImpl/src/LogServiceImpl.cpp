#include <sstream>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "cppmicroservices/ServiceReference.h"

#include "LogServiceImpl.hpp"
#include "LoggerFactoryImpl.hpp"
#include "LoggerImpl.hpp"

namespace cppmicroservices::logservice
{     
        LogServiceImpl::LogServiceImpl(std::string const& loggerName)
        {
            logger = getLogger(loggerName);    
	}
   
        void
        LogServiceImpl::Log(SeverityLevel level, std::string const& message)
        {

           auto currLogger = std::atomic_load(&logger);
           if (!currLogger)
           {
               return;
           }

	   logImpl(level, message);
	}

        void
        LogServiceImpl::Log(SeverityLevel level, std::string const& message, const std::exception_ptr ex)
        {
            auto currLogger = std::atomic_load(&logger);
            if (!currLogger)
            {
               return;
            }

	    logImpl(level, message, ex);
	}

	void
	LogServiceImpl::Log(ServiceReferenceBase const& sr, SeverityLevel level, std::string const& message)
	{
	    auto currLogger = std::atomic_load(&logger);
	    if (!currLogger)
	    {
		return;
	    }

	    logImpl(level, message, sr);
	}

	void
	LogServiceImpl::Log(ServiceReferenceBase const& sr,
				SeverityLevel level,
				std::string const& message,
				const std::exception_ptr ex)
	{

	    auto currLogger = std::atomic_load(&logger);
	    if(!currLogger)
	    {
		return;
       	    }

	    logImpl(level, message, sr, ex);
	}

        std::shared_ptr<Logger> 
            LogServiceImpl::getLogger(std::string const& name) const
        {
            std::shared_ptr<LoggerFactory> lf = std::make_shared<LoggerFactoryImpl>();
            return lf->getLogger(name);
        }

	std::shared_ptr<Logger>
            LogServiceImpl::getLogger(const cppmicroservices::Bundle& bundle, std::string const& name) const
        {
            std::shared_ptr<LoggerFactory> lf = std::make_shared<LoggerFactoryImpl>();
            return lf->getLogger(bundle, name);
        }

        void
        LogServiceImpl::AddSink(spdlog::sink_ptr& sink)
        {
	    auto currLogger = std::atomic_load(&logger);
            if (!currLogger)
            {
                return;
            }
	    std::shared_ptr<LoggerImpl> logimpl = std::dynamic_pointer_cast<LoggerImpl>(currLogger);
            logimpl->AddSink(sink);
        }
} // namespace cppmicroservices::logservice
