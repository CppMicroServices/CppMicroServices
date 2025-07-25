#include <sstream>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "cppmicroservices/ServiceReference.h"

#include "LoggerImpl.hpp"

namespace cppmicroservices::logservice
{
        inline std::string
        GetExceptionMessage(std::exception_ptr const& ex)
        {
            std::string message = "\nException logged: ";
            if (ex)
            {
                try
                {
                    std::rethrow_exception(ex);
                }
                catch (std::exception const& e)
                {
                    message += std::string(typeid(e).name()) + " : " + e.what();
                }
            }
            else
            {
                message += "none";
            }

            return message;
        }

        inline std::string
        GetServiceReferenceInfo(ServiceReferenceBase const& sr)
        {
            std::ostringstream stream;
            stream << "\nServiceReference: " << sr;
            return stream.str();
        }

        LoggerImpl::LoggerImpl(std::string const& loggerName)
        {
            auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            m_Logger = std::make_shared<spdlog::logger>(loggerName, std::move(sink));
            m_Logger->set_pattern("[%T] [%P:%t] %n (%^%l%$): %v");
            m_Logger->set_level(spdlog::level::trace);
        }

	LoggerImpl::LoggerImpl(const cppmicroservices::Bundle& bundle, std::string const& loggerName)
        {
            auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	    const std::string logger_name = bundle.GetSymbolicName() + "." + loggerName;
            m_Logger = std::make_shared<spdlog::logger>(logger_name, std::move(sink));
            m_Logger->set_pattern("[%T] [%P:%t] %n (%^%l%$): %v");
            m_Logger->set_level(spdlog::level::trace);
        }

        void
        LoggerImpl::audit(std::string const& message)
        {
	    m_Logger->set_level(spdlog::level::trace);
	    m_Logger->trace(message);
        }

        void 
        LoggerImpl::audit(std::string const& format, std::string const& arg)
        {
            m_Logger->set_level(spdlog::level::trace);
            m_Logger->trace(format, arg);
        }

        void
        LoggerImpl::audit(std::string const& format, std::string const& arg1, std::string const& arg2)
        {
            m_Logger->set_level(spdlog::level::trace);
            m_Logger->trace(format, arg1, arg2);
        }

        void
        LoggerImpl::audit(std::string const& message, const std::exception_ptr ex)
        {
            m_Logger->set_level(spdlog::level::trace);
            m_Logger->trace(message + GetExceptionMessage(ex));
        }
        void
        LoggerImpl::audit(std::string const& message, ServiceReferenceBase const& sr)
        {
            m_Logger->set_level(spdlog::level::trace);
            m_Logger->trace(message + GetServiceReferenceInfo(sr));
        }

        void
        LoggerImpl::audit(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex)
        {
            m_Logger->set_level(spdlog::level::trace);
            m_Logger->trace(message + GetServiceReferenceInfo(sr) + GetExceptionMessage(ex));
        }

        void
        LoggerImpl::debug(std::string const& message)
        {
            m_Logger->set_level(spdlog::level::debug);
            m_Logger->debug(message);
        }

        void
        LoggerImpl::debug(std::string const& format, std::string const& arg)
        {
            m_Logger->set_level(spdlog::level::debug);
            m_Logger->debug(format, arg);
        }

        void
        LoggerImpl::debug(std::string const& format, std::string const& arg1, std::string const& arg2)
        {
            m_Logger->set_level(spdlog::level::debug);
            m_Logger->debug(format, arg1, arg2);
        }

	void
        LoggerImpl::debug(std::string const& message, const std::exception_ptr ex)
	{
	    m_Logger->set_level(spdlog::level::debug);
	    m_Logger->debug(message + GetExceptionMessage(ex));
	}
        void
	LoggerImpl::debug(std::string const& message, ServiceReferenceBase const& sr)
	{
	    m_Logger->set_level(spdlog::level::debug);
            m_Logger->debug(message + GetServiceReferenceInfo(sr));
	}

        void 
	LoggerImpl::debug(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex)
	{
	     m_Logger->set_level(spdlog::level::debug);
             m_Logger->debug(message + GetServiceReferenceInfo(sr) + GetExceptionMessage(ex));
	}

        void
        LoggerImpl::error(std::string const& message)
        {
            m_Logger->set_level(spdlog::level::err);
            m_Logger->error(message);
        }

        void
        LoggerImpl::error(std::string const& format, std::string const& arg)
        {
            m_Logger->set_level(spdlog::level::err);
            m_Logger->error(format, arg);
        }

        void
        LoggerImpl::error(std::string const& format, std::string const& arg1, std::string const& arg2)
        {
            m_Logger->set_level(spdlog::level::err);
            m_Logger->error(format, arg1, arg2);
        }

        void
        LoggerImpl::error(std::string const& message, const std::exception_ptr ex)
        {
            m_Logger->set_level(spdlog::level::err);
            m_Logger->error(message + GetExceptionMessage(ex));
        }
        void
        LoggerImpl::error(std::string const& message, ServiceReferenceBase const& sr)
        {
            m_Logger->set_level(spdlog::level::err);
            m_Logger->error(message + GetServiceReferenceInfo(sr));
        }

        void
        LoggerImpl::error(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex)
        {
            m_Logger->set_level(spdlog::level::err);
            m_Logger->error(message + GetServiceReferenceInfo(sr) + GetExceptionMessage(ex));
        }

        void
        LoggerImpl::info(std::string const& message)
        {
            m_Logger->set_level(spdlog::level::info);
            m_Logger->info(message);
        }

        void
        LoggerImpl::info(std::string const& format, std::string const& arg)
        {
            m_Logger->set_level(spdlog::level::info);
            m_Logger->info(format, arg);
        }

        void
        LoggerImpl::info(std::string const& format, std::string const& arg1, std::string const& arg2)
        {
            m_Logger->set_level(spdlog::level::info);
            m_Logger->info(format, arg1, arg2);
        }

          void
        LoggerImpl::info(std::string const& message, const std::exception_ptr ex)
        {
            m_Logger->set_level(spdlog::level::info);
            m_Logger->info(message + GetExceptionMessage(ex));
        }
        void
        LoggerImpl::info(std::string const& message, ServiceReferenceBase const& sr)
        {
            m_Logger->set_level(spdlog::level::info);
            m_Logger->info(message + GetServiceReferenceInfo(sr));
        }

        void
        LoggerImpl::info(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex)
        {
            m_Logger->set_level(spdlog::level::info);
            m_Logger->info(message + GetServiceReferenceInfo(sr) + GetExceptionMessage(ex));
        }

        void
        LoggerImpl::trace(std::string const& message)
        {
            m_Logger->set_level(spdlog::level::trace);
            m_Logger->trace(message);
        }

        void
        LoggerImpl::trace(std::string const& format, std::string const& arg)
        {
            m_Logger->set_level(spdlog::level::trace);
            m_Logger->trace(format, arg);
        }

        void
        LoggerImpl::trace(std::string const& format, std::string const& arg1, std::string const& arg2)
        {
            m_Logger->set_level(spdlog::level::trace);
            m_Logger->trace(format, arg1, arg2);
        }

          void
        LoggerImpl::trace(std::string const& message, const std::exception_ptr ex)
        {
            m_Logger->set_level(spdlog::level::trace);
            m_Logger->trace(message + GetExceptionMessage(ex));
        }
        void
        LoggerImpl::trace(std::string const& message, ServiceReferenceBase const& sr)
        {
            m_Logger->set_level(spdlog::level::trace);
            m_Logger->trace(message + GetServiceReferenceInfo(sr));
        }

        void
        LoggerImpl::trace(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex)
        {
            m_Logger->set_level(spdlog::level::trace);
            m_Logger->trace(message + GetServiceReferenceInfo(sr) + GetExceptionMessage(ex));
        }

        void
        LoggerImpl::warn(std::string const& message)
        {
            m_Logger->set_level(spdlog::level::warn);
            m_Logger->warn(message);
        }

        void
        LoggerImpl::warn(std::string const& format, std::string const& arg)
        {
            m_Logger->set_level(spdlog::level::warn);
            m_Logger->warn(format, arg);
        }

        void
        LoggerImpl::warn(std::string const& format, std::string const& arg1, std::string const& arg2)
        {
            m_Logger->set_level(spdlog::level::warn);
            m_Logger->warn(format, arg1, arg2);
        }

        void
        LoggerImpl::warn(std::string const& message, const std::exception_ptr ex)
        {
            m_Logger->set_level(spdlog::level::warn);
            m_Logger->warn(message + GetExceptionMessage(ex));
        }
        void
        LoggerImpl::warn(std::string const& message, ServiceReferenceBase const& sr)
        {
            m_Logger->set_level(spdlog::level::warn);
            m_Logger->warn(message + GetServiceReferenceInfo(sr));
        }

        void
        LoggerImpl::warn(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex)
        {
            m_Logger->set_level(spdlog::level::warn);
            m_Logger->warn(message + GetServiceReferenceInfo(sr) + GetExceptionMessage(ex));
        }
        void
        LoggerImpl::AddSink(spdlog::sink_ptr& sink)
        {
            m_Logger->sinks().push_back(sink);
        }

} // namespace cppmicroservices::logservice
