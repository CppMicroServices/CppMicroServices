#include "cppmicroservices/logservice/LoggerFactory.hpp"
#include <mutex>

namespace cppmicroservices
{
    namespace logservice
    {
        class LoggerFactoryImpl final : public LoggerFactory
        {
          public:
              LoggerFactoryImpl() = default;
              ~LoggerFactoryImpl() = default;

              std::shared_ptr<Logger> getLogger(std::string const& name) const override;
              std::shared_ptr<Logger> getLogger(cppmicroservices::Bundle bundle, std::string const& name) const override;              

	  private:
	      mutable std::mutex mutex; // Mutex for synchronization
        };
    } // namespace logservice
} // namespace cppmicroservices
