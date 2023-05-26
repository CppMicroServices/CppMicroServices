#include "TestInterfaces/Interfaces.hpp"

#include "ManagedServiceFactoryServiceImpl3.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/cm/ManagedServiceFactory.hpp>

#include <map>
#include <memory>
#include <mutex>

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                class TestManagedServiceFactoryImpl3
                    : public ::test::TestManagedServiceFactory
                    , public ::cppmicroservices::service::cm::ManagedServiceFactory
                {

                  public:
                    virtual ~TestManagedServiceFactoryImpl3();

                    void Activate(
                        std::shared_ptr<cppmicroservices::service::component::ComponentContext> const& context);

                    void Updated(std::string const& pid, AnyMap const& properties) override;

                    void Removed(std::string const& pid) override;

                    int getUpdatedCounter(std::string const& pid) override;

                    int getRemovedCounter(std::string const& pid) override;

                    std::shared_ptr<::test::TestManagedServiceFactoryServiceInterface> create(
                        std::string const& config) override;

                  private:
                    std::map<std::string, int> m_updatedCallCount;
                    std::map<std::string, int> m_removedCallCount;
                    cppmicroservices::BundleContext bundleContext_;
                    std::mutex m_updatedMtx;
                    std::mutex m_removedMtx;
                };

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
