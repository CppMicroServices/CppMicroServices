#include <sstream>

#include "spdlog/pattern_formatter.h"
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

        namespace
        {
            // Audit records are logged at spdlog's highest level, which nothing else uses.
            // A level filter on the logger or on a sink therefore never drops them.
            constexpr auto AuditLevel = spdlog::level::critical;

            // Prints the level name like %l does, except that audit records read "audit".
            class LevelNameFlag final : public spdlog::custom_flag_formatter
            {
              public:
                void
                format(spdlog::details::log_msg const& msg, std::tm const&, spdlog::memory_buf_t& dest) override
                {
                    static spdlog::string_view_t const audit("audit");
                    auto const name = (msg.level == AuditLevel) ? audit : spdlog::level::to_string_view(msg.level);
                    dest.append(name.data(), name.data() + name.size());
                }

                std::unique_ptr<spdlog::custom_flag_formatter>
                clone() const override
                { return spdlog::details::make_unique<LevelNameFlag>(); }
            };

            std::shared_ptr<spdlog::logger>
            MakeSpdLogger(std::string const& loggerName)
            {
                auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                auto logger = std::make_shared<spdlog::logger>(loggerName, std::move(sink));
                logger->set_formatter(LoggerImpl::MakeFormatter());
                // The level is set once here. Logging calls must not change it: the logger is
                // shared between threads, and a message is dropped if another thread raises
                // the level between a caller's set_level and its log call.
                logger->set_level(spdlog::level::trace);
                return logger;
            }
        } // namespace

        std::unique_ptr<spdlog::formatter>
        LoggerImpl::MakeFormatter()
        {
            auto formatter = spdlog::details::make_unique<spdlog::pattern_formatter>();
            formatter->add_flag<LevelNameFlag>('*').set_pattern("[%T] [%P:%t] %n (%^%*%$): %v");
            return formatter;
        }

        LoggerImpl::LoggerImpl(std::string const& loggerName) : m_Logger(MakeSpdLogger(loggerName)) {}

        LoggerImpl::LoggerImpl(cppmicroservices::Bundle const& bundle, std::string const& loggerName)
            : m_Logger(MakeSpdLogger(bundle.GetSymbolicName() + "." + loggerName))
        {
        }

        void
        LoggerImpl::audit(std::string const& message)
        { m_Logger->log(AuditLevel, message); }

        void 
        LoggerImpl::audit(std::string const& format, std::string const& arg)
        { m_Logger->log(AuditLevel, format, arg); }

        void
        LoggerImpl::audit(std::string const& format, std::string const& arg1, std::string const& arg2)
        { m_Logger->log(AuditLevel, format, arg1, arg2); }

        void
        LoggerImpl::audit(std::string const& message, const std::exception_ptr ex)
        { m_Logger->log(AuditLevel, message + GetExceptionMessage(ex)); }
        void
        LoggerImpl::audit(std::string const& message, ServiceReferenceBase const& sr)
        { m_Logger->log(AuditLevel, message + GetServiceReferenceInfo(sr)); }

        void
        LoggerImpl::audit(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex)
        { m_Logger->log(AuditLevel, message + GetServiceReferenceInfo(sr) + GetExceptionMessage(ex)); }

        void
        LoggerImpl::debug(std::string const& message)
        {
            m_Logger->debug(message);
        }

        void
        LoggerImpl::debug(std::string const& format, std::string const& arg)
        {
            m_Logger->debug(format, arg);
        }

        void
        LoggerImpl::debug(std::string const& format, std::string const& arg1, std::string const& arg2)
        {
            m_Logger->debug(format, arg1, arg2);
        }

	void
        LoggerImpl::debug(std::string const& message, const std::exception_ptr ex)
	{
	    m_Logger->debug(message + GetExceptionMessage(ex));
	}
        void
	LoggerImpl::debug(std::string const& message, ServiceReferenceBase const& sr)
	{
            m_Logger->debug(message + GetServiceReferenceInfo(sr));
	}

        void 
	LoggerImpl::debug(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex)
	{
             m_Logger->debug(message + GetServiceReferenceInfo(sr) + GetExceptionMessage(ex));
	}

        void
        LoggerImpl::error(std::string const& message)
        {
            m_Logger->error(message);
        }

        void
        LoggerImpl::error(std::string const& format, std::string const& arg)
        {
            m_Logger->error(format, arg);
        }

        void
        LoggerImpl::error(std::string const& format, std::string const& arg1, std::string const& arg2)
        {
            m_Logger->error(format, arg1, arg2);
        }

        void
        LoggerImpl::error(std::string const& message, const std::exception_ptr ex)
        {
            m_Logger->error(message + GetExceptionMessage(ex));
        }
        void
        LoggerImpl::error(std::string const& message, ServiceReferenceBase const& sr)
        {
            m_Logger->error(message + GetServiceReferenceInfo(sr));
        }

        void
        LoggerImpl::error(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex)
        {
            m_Logger->error(message + GetServiceReferenceInfo(sr) + GetExceptionMessage(ex));
        }

        void
        LoggerImpl::info(std::string const& message)
        {
            m_Logger->info(message);
        }

        void
        LoggerImpl::info(std::string const& format, std::string const& arg)
        {
            m_Logger->info(format, arg);
        }

        void
        LoggerImpl::info(std::string const& format, std::string const& arg1, std::string const& arg2)
        {
            m_Logger->info(format, arg1, arg2);
        }

          void
        LoggerImpl::info(std::string const& message, const std::exception_ptr ex)
        {
            m_Logger->info(message + GetExceptionMessage(ex));
        }
        void
        LoggerImpl::info(std::string const& message, ServiceReferenceBase const& sr)
        {
            m_Logger->info(message + GetServiceReferenceInfo(sr));
        }

        void
        LoggerImpl::info(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex)
        {
            m_Logger->info(message + GetServiceReferenceInfo(sr) + GetExceptionMessage(ex));
        }

        void
        LoggerImpl::trace(std::string const& message)
        {
            m_Logger->trace(message);
        }

        void
        LoggerImpl::trace(std::string const& format, std::string const& arg)
        {
            m_Logger->trace(format, arg);
        }

        void
        LoggerImpl::trace(std::string const& format, std::string const& arg1, std::string const& arg2)
        {
            m_Logger->trace(format, arg1, arg2);
        }

          void
        LoggerImpl::trace(std::string const& message, const std::exception_ptr ex)
        {
            m_Logger->trace(message + GetExceptionMessage(ex));
        }
        void
        LoggerImpl::trace(std::string const& message, ServiceReferenceBase const& sr)
        {
            m_Logger->trace(message + GetServiceReferenceInfo(sr));
        }

        void
        LoggerImpl::trace(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex)
        {
            m_Logger->trace(message + GetServiceReferenceInfo(sr) + GetExceptionMessage(ex));
        }

        void
        LoggerImpl::warn(std::string const& message)
        {
            m_Logger->warn(message);
        }

        void
        LoggerImpl::warn(std::string const& format, std::string const& arg)
        {
            m_Logger->warn(format, arg);
        }

        void
        LoggerImpl::warn(std::string const& format, std::string const& arg1, std::string const& arg2)
        {
            m_Logger->warn(format, arg1, arg2);
        }

        void
        LoggerImpl::warn(std::string const& message, const std::exception_ptr ex)
        {
            m_Logger->warn(message + GetExceptionMessage(ex));
        }
        void
        LoggerImpl::warn(std::string const& message, ServiceReferenceBase const& sr)
        {
            m_Logger->warn(message + GetServiceReferenceInfo(sr));
        }

        void
        LoggerImpl::warn(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex)
        {
            m_Logger->warn(message + GetServiceReferenceInfo(sr) + GetExceptionMessage(ex));
        }
        void
        LoggerImpl::AddSink(spdlog::sink_ptr& sink)
        {
            m_Logger->sinks().push_back(sink);
        }

} // namespace cppmicroservices::logservice
