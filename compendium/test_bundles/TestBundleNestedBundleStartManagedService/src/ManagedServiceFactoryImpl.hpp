#include "TestInterfaces/Interfaces.hpp"

#include <cppmicroservices/cm/ManagedServiceFactory.hpp>
#include <cppmicroservices/BundleContext.h>
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

#include <map>
#include <memory>
#include <mutex>

namespace cppmicroservices {
namespace service {
namespace cm {
namespace test {

class TestManagedServiceFactoryImpl
  : public ::cppmicroservices::service::cm::ManagedServiceFactory
{

public:
  virtual ~TestManagedServiceFactoryImpl();

  void Activate(const std::shared_ptr<cppmicroservices::service::component::ComponentContext>& context);
  void Updated(std::string const& pid, AnyMap const& properties) override;
  void Removed(std::string const& pid) override;

private:
  std::map<std::string, int> m_updatedCallCount;
  std::map<std::string, int> m_removedCallCount;
  std::mutex m_updatedMtx;
  std::mutex m_removedMtx;
};

} // namespace test
} // namespace cm
} // namespace service
} // namespace cppmicroservices
