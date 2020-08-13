#include <sstream>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "cppmicroservices/ServiceReference.h"

#include "LogServiceImpl.hpp"

namespace cppmicroservices {
namespace logservice {
std::string GetExceptionMessage(const std::exception_ptr& ex)
{
  std::string message = "\nException logged: ";
  if (ex) {
    std::ostringstream stream;
    try {
      std::rethrow_exception(ex);
    } catch (const std::exception& e) {
      stream << typeid(e).name() << " : " << e.what();
    }
    message += stream.str();
  } else {
    message += "none";
  }

  return message;
}

std::string GetServiceReferenceInfo(const ServiceReferenceBase& sr)
{
  std::ostringstream stream;
  stream << "\nServiceReference: " << sr;
  return stream.str();
}

LogServiceImpl::LogServiceImpl(const std::string& loggerName)
{
  auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  m_Logger = std::make_shared<spdlog::logger>(loggerName, std::move(sink));
  m_Logger->set_pattern("[%T] [%P:%t] %n (%^%l%$): %v");
  m_Logger->set_level(spdlog::level::trace);
}

void LogServiceImpl::Log(SeverityLevel level, const std::string& message)
{
  switch (level) {
    case SeverityLevel::LOG_DEBUG: {
      m_Logger->debug(message);
      break;
    }
    case SeverityLevel::LOG_INFO: {
      m_Logger->info(message);
      break;
    }
    case SeverityLevel::LOG_WARNING: {
      m_Logger->warn(message);
      break;
    }
    case SeverityLevel::LOG_ERROR: {
      m_Logger->error(message);
      break;
    }
  }
}

void LogServiceImpl::Log(SeverityLevel level,
                         const std::string& message,
                         const std::exception_ptr ex)
{
  std::string full_message = message;
  full_message = message + GetExceptionMessage(ex);
  LogServiceImpl::Log(level, full_message);
}

void LogServiceImpl::Log(const ServiceReferenceBase& sr,
                         SeverityLevel level,
                         const std::string& message)
{
  std::string full_message = message;
  full_message = message + GetServiceReferenceInfo(sr);
  LogServiceImpl::Log(level, full_message);
}

void LogServiceImpl::Log(const ServiceReferenceBase& sr,
                         SeverityLevel level,
                         const std::string& message,
                         const std::exception_ptr ex)
{
  std::string full_message = message;
  full_message = message + GetServiceReferenceInfo(sr) + GetExceptionMessage(ex);
  LogServiceImpl::Log(level, full_message);
}

void LogServiceImpl::AddSink(spdlog::sink_ptr& sink)
{
  m_Logger->sinks().push_back(sink);
}
}
}
