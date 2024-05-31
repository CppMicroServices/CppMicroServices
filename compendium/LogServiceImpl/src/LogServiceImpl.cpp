#include <sstream>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "cppmicroservices/ServiceReference.h"

#include "LogServiceImpl.hpp"
#include "LoggerFactoryImpl.hpp"

namespace cppmicroservices::logservice
{     
        LogServiceImpl::LogServiceImpl(std::string const& loggerName)
        {
            logger = getLogger(loggerName);    
	}

        void
        LogServiceImpl::Log(SeverityLevel level, std::string const& message)
        {
            switch (level)
            {
                case SeverityLevel::LOG_DEBUG:
                {
		    auto currLogger = std::atomic_load(&logger);

		    if(currLogger)
		    {
                        currLogger->debug(message);
		    }
                    break;
                }
                case SeverityLevel::LOG_INFO:
                {
                     auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
		        currLogger->info(message);
		    }
                    break;
                }
                case SeverityLevel::LOG_WARNING:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
                       currLogger->warn(message);
		    }
                    break;
                }
                case SeverityLevel::LOG_ERROR:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
                       currLogger->error(message);
		    }
                    break;
                }
		default:
		{
			auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
		      currLogger->trace(message);
		    }
		    break;
		}
            }
        }

        void
        LogServiceImpl::Log(SeverityLevel level, std::string const& message, const std::exception_ptr ex)
        {
            switch (level)
            {
                case SeverityLevel::LOG_DEBUG:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {	
                       currLogger->debug(message, ex);
		    }
                    break;
                }
                case SeverityLevel::LOG_INFO:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
                       currLogger->info(message, ex);
		    }
                    break;
                }
                case SeverityLevel::LOG_WARNING:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
                        currLogger->warn(message, ex);
		    }
                    break;
                }
                case SeverityLevel::LOG_ERROR:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
                        currLogger->error(message, ex);
		    }
                    break;
                }
		default:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
                        currLogger->trace(message, ex);
		    }
                    break;
                }
            }
        }

        void
        LogServiceImpl::Log(ServiceReferenceBase const& sr, SeverityLevel level, std::string const& message)
        {
            switch (level)
            {
                case SeverityLevel::LOG_DEBUG:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
                       currLogger->debug(message, sr);
		    }
                    break;
                }
                case SeverityLevel::LOG_INFO:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
                        currLogger->info(message, sr);
		    }
                    break;
                }
                case SeverityLevel::LOG_WARNING:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
                        currLogger->warn(message, sr);
		    }
                    break;
                }
                case SeverityLevel::LOG_ERROR:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
                       currLogger->error(message, sr);
		    }
                    break;
                }
		default:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
                        currLogger->trace(message, sr);
		    }
                    break;
                }
            }
        }

        void
        LogServiceImpl::Log(ServiceReferenceBase const& sr,
                            SeverityLevel level,
                            std::string const& message,
                            const std::exception_ptr ex)
        {
            switch (level)
            {
                case SeverityLevel::LOG_DEBUG:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
                        currLogger->debug(message, sr, ex);
		    }
                    break;
                }
                case SeverityLevel::LOG_INFO:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
                        currLogger->info(message, sr, ex);
		    }
                    break;
                }
                case SeverityLevel::LOG_WARNING:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
                        currLogger->warn(message, sr, ex);
		    }
                    break;
                }
                case SeverityLevel::LOG_ERROR:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
                       currLogger->error(message, sr, ex);
		    }
                    break;
                }
		default:
                {
			 auto currLogger = std::atomic_load(&logger);

                    if(currLogger)
		    {
                       currLogger->trace(message, sr, ex);
		    }
                    break;
                }
            }
        }

        std::shared_ptr<Logger> 
            LogServiceImpl::getLogger(std::string const& name) const
        {
            std::shared_ptr<LoggerFactory> lf = std::make_shared<LoggerFactoryImpl>();
            return lf->getLogger(name);
        }

	std::shared_ptr<Logger>
            LogServiceImpl::getLogger(cppmicroservices::Bundle bundle, std::string const& name) const
        {
            std::shared_ptr<LoggerFactory> lf = std::make_shared<LoggerFactoryImpl>();
            return lf->getLogger(bundle, name);
        }

        void
        LogServiceImpl::AddSink(spdlog::sink_ptr& sink)
        {
            logger->AddSink(sink);
        }
} // namespace cppmicroservices::logservice
