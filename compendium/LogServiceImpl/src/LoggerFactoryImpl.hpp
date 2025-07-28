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
              ~LoggerFactoryImpl() override = default;

	       // Copy constructor
               LoggerFactoryImpl(const LoggerFactoryImpl& other) = delete;

               // Copy assignment operator
               LoggerFactoryImpl& operator=(const LoggerFactoryImpl& other) = delete;

               // Move constructor
               LoggerFactoryImpl(LoggerFactoryImpl&& other) noexcept = delete;

               // Move assignment operator
               LoggerFactoryImpl& operator=(LoggerFactoryImpl&& other) noexcept = delete;


              [[nodiscard]] std::shared_ptr<Logger> getLogger(std::string const& name) const override;
              [[nodiscard]] std::shared_ptr<Logger> getLogger(const cppmicroservices::Bundle& bundle, std::string const& name) const override;              

	  private:
	      mutable std::mutex mutex; // Mutex for synchronization
        };
} // namespace logservice
} // namespace cppmicroservices
