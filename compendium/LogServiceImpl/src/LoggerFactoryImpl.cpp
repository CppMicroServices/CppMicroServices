#include "LoggerFactoryImpl.hpp"
#include "LoggerImpl.hpp"

namespace cppmicroservices::logservice
{
        std::shared_ptr<Logger> 
        LoggerFactoryImpl::getLogger(const std::string& name) const 
	{
	    std::lock_guard<std::mutex> lock(mutex);
            return std::make_shared<LoggerImpl>(name);
	}

	std::shared_ptr<Logger> 
        LoggerFactoryImpl::getLogger(const cppmicroservices::Bundle& bundle, std::string const& name) const 
	{
	    std::lock_guard<std::mutex> lock(mutex);
	    return std::make_shared<LoggerImpl>(bundle, name);
	}
    
}

