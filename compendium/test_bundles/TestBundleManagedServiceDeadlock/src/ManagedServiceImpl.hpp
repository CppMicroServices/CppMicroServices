#include <cppmicroservices/AnyMap.h>
#include <cppmicroservices/cm/ManagedService.hpp>

#include "TestInterfaces/Interfaces.hpp"

#include <mutex>

namespace cppmicroservices {
namespace service {
namespace cm {
namespace test {

class TestManagedServiceImpl2
  : public ::test::TestManagedServiceInterface
  , public cppmicroservices::service::cm::ManagedService
{
public:
  TestManagedServiceImpl2();

  virtual ~TestManagedServiceImpl2();

  void Updated(AnyMap const& properties) override;

  int getCounter() override;

private:
  int m_counter;
  std::mutex m_counterMtx;
  std::mutex m_propertiesMutex;
  std::shared_ptr<std::mutex> m_cvMtx;
  std::shared_ptr<std::condition_variable> m_cv;
};

} // namespace test
} // namespace cm
} // namespace service
} // namespace cppmicroservices
