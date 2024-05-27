#include "cppmicroservices/logservice/Logger.hpp"
#include "cppmicroservices/Bundle.h"

namespace cppmicroservices
{
    namespace logservice
    {
        class LoggerImpl final : public Logger
        {
	    public:
                LoggerImpl(std::string const& loggerName);
		LoggerImpl(cppmicroservices::Bundle bundle, std::string const& loggerName);
                ~LoggerImpl() = default;

	         void audit(const std::string& message) override;
                 void audit(std::string const& format, std::string const& arg) override;
                 void audit(std::string const& format, std::string const& arg1, std::string const& arg2) override;
                void audit(std::string const& message, const std::exception_ptr ex) override;
                void audit(std::string const& message, ServiceReferenceBase const& sr) override;
                void audit(std::string const& message,
                           ServiceReferenceBase const& sr,
                           const std::exception_ptr ex) override;

               
                 void debug(std::string const& message) override;
                 void debug(std::string const& format, std::string const& arg) override;
                 void debug(std::string const& format, std::string const& arg1, std::string const& arg2) override;
		 void debug(std::string const& message, const std::exception_ptr ex) override;
                 void debug(std::string const& message, ServiceReferenceBase const& sr) override;
                 void debug(std::string const& message, ServiceReferenceBase const& sr, const std::exception_ptr ex) override;


                 void error(std::string const& message) override;
            	 void error(std::string const& format, std::string const& arg) override;
            	 void error(std::string const& format, std::string const& arg1, std::string const& arg2) override;
                 void error(std::string const& message, const std::exception_ptr ex) override;
                 void error(std::string const& message, ServiceReferenceBase const& sr) override;
                 void error(std::string const& message,
                            ServiceReferenceBase const& sr,
                            const std::exception_ptr ex) override;
                 
            	 void info(std::string const& message) override;
            	 void info(std::string const& format, std::string const& arg) override;
            	 void info(std::string const& format, std::string const& arg1, std::string const& arg2) override;
                 void info(std::string const& message, const std::exception_ptr ex) override;
                 void info(std::string const& message, ServiceReferenceBase const& sr) override;
                 void info(std::string const& message,
                            ServiceReferenceBase const& sr,
                            const std::exception_ptr ex) override;

            	 void trace(std::string const& message) override;
            	 void trace(std::string const& format, std::string const& arg) override;
            	 void trace(std::string const& format, std::string const& arg1, std::string const& arg2) override;
                 void trace(std::string const& message, const std::exception_ptr ex) override;
                 void trace(std::string const& message, ServiceReferenceBase const& sr) override;
                 void trace(std::string const& message,
                            ServiceReferenceBase const& sr,
                            const std::exception_ptr ex) override;

            	 void warn(std::string const& message) override;
            	 void warn(std::string const& format, std::string const& arg) override;
               	 void warn(std::string const& format, std::string const& arg1, std::string const& arg2) override;
                 void warn(std::string const& message, const std::exception_ptr ex) override;
                 void warn(std::string const& message, ServiceReferenceBase const& sr) override;
                 void warn(std::string const& message,
                            ServiceReferenceBase const& sr,
                            const std::exception_ptr ex) override;

                 void AddSink(spdlog::sink_ptr& sink) override;
 
             private:
                 std::shared_ptr<::spdlog::logger> m_Logger;
        };
    } // namespace logservice
} // namespace cppmicroservices
