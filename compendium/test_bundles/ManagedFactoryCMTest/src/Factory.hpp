#include "TestInterfaces/Interfaces.hpp"

#include "FactoryServiceImpl.hpp"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/cm/ManagedServiceFactory.hpp>
namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                class TestManagedFactoryImpl : public ::cppmicroservices::service::cm::ManagedServiceFactory
                {
                  public:
                    virtual ~TestManagedFactoryImpl();

                    void Updated(std::string const& pid, AnyMap const& properties) override;

                    void Removed(std::string const& pid) override;

                    void Activate(
                        std::shared_ptr<cppmicroservices::service::component::ComponentContext> const& context);

                    void Deactivate(
                        std::shared_ptr<cppmicroservices::service::component::ComponentContext> const& context);

                  private:
                    std::unordered_map<
                        std::string,
                        cppmicroservices::ServiceRegistration<::test::TestManagedServiceFactoryServiceInterface>>
                        regs;
                    BundleContext bc;
                };
            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
