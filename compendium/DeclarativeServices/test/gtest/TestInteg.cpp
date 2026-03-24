#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>

#include "../TestUtils.hpp"
#include "ConcurrencyTestUtil.hpp"
#include "Mocks.hpp"
#include "TestFixture.hpp"
#include "TestInterfaces/Interfaces.hpp"

#include <atomic>
#include <chrono>
#include <future>
#include <thread>
#include <vector>

// Add these test classes and helper classes

namespace test
{
    // Helper class for DSGraph05 test
    class basicDS5 : public test::DSGraph05
    {
      public:
        basicDS5() = default;
        ~basicDS5() override = default;
        std::string
        Description() override
        {
            return "basicDS5";
        };
    };

    // Test fixture for integration tests
    class IntegrationTestFixture : public ::testing::Test
    {
      public:
        IntegrationTestFixture() : framework(cppmicroservices::FrameworkFactory().NewFramework()) {}

        void
        SetUp() override
        {
            framework.Start();
            auto context = framework.GetBundleContext();

            ::test::InstallAndStartDS(context);
            ::test::InstallAndStartConfigAdmin(context);
        }

        void
        TearDown() override
        {
            framework.Stop();
            framework.WaitForStop(std::chrono::milliseconds::zero());
        }

        std::shared_ptr<cppmicroservices::service::component::runtime::ServiceComponentRuntime> dsRuntimeService;
        cppmicroservices::Framework framework;
    };

    /**
     * Verify that ServiceReferenceFromService works for services provided by a
     * ServiceFactory. A ServiceFactory-created service is bound to a dependent
     * DS component via a bind method; the bind method calls
     * ServiceReferenceFromService on the injected shared_ptr and expects it to
     * succeed. The test also verifies that two external GetService calls for the
     * same factory service return ServiceReferences that compare equal.
     */
    TEST_F(IntegrationTestFixture, testServiceReferenceFromServiceFactory)
    {
        auto const& param = std::make_shared<AsyncWorkServiceThreadPool>(10);
        auto context = framework.GetBundleContext();

        // ASYNCWORKSERVICE
        auto reg = context.RegisterService<cppmicroservices::async::AsyncWorkService>(param);

        // Start bundle
        std::string componentName = "sample::ServiceComponentCA10";

        ::test::InstallAndStartBundle(context, "ServiceFactoryBundle");
        ::test::InstallAndStartBundle(context, "ServiceFactoryDependentBundle");

        auto factoryRef = context.GetServiceReferences<cppmicroservices::ServiceFactory>("(mySvc=true)");
        auto factorySvc = context.GetService<cppmicroservices::ServiceFactory>(factoryRef.front());

        ASSERT_TRUE(factorySvc);

        // register the factory
        context.RegisterService<test::FactoryCreatedService>(std::static_pointer_cast<cppmicroservices::ServiceFactory>(factorySvc));

        // grab the dependent service, make sure it bound succesfully
        auto dependentSvcRef = context.GetServiceReference<test::FactoryServiceDependent>();
        auto dependentSvc = context.GetService<test::FactoryServiceDependent>(dependentSvcRef);
        ASSERT_TRUE(dependentSvc && dependentSvc->didBind());

        // verify that two calls to getServiceFromReference will return the same service, with the right interfaceid
        ASSERT_EQ(cppmicroservices::ServiceReferenceFromService(
                      context.GetService<test::FactoryCreatedService>(
                          context.GetServiceReference<test::FactoryCreatedService>()))
                      .GetInterfaceId(),
                  "test::FactoryCreatedService");
        ASSERT_EQ(cppmicroservices::ServiceReferenceFromService(context.GetService<test::FactoryCreatedService>(
                      context.GetServiceReference<test::FactoryCreatedService>())),
                  cppmicroservices::ServiceReferenceFromService(context.GetService<test::FactoryCreatedService>(
                      context.GetServiceReference<test::FactoryCreatedService>())));
    }
} // namespace test