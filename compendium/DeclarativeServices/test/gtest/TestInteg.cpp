#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/servicecomponent/ComponentConstants.hpp>
#include <cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp>

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
        { return "basicDS5"; };
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

            auto sRef
                = context.GetServiceReference<cppmicroservices::service::component::runtime::ServiceComponentRuntime>();
            ASSERT_TRUE(sRef);
            dsRuntimeService
                = context.GetService<cppmicroservices::service::component::runtime::ServiceComponentRuntime>(sRef);
            ASSERT_TRUE(dsRuntimeService);
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
        context.RegisterService<test::FactoryCreatedService>(
            std::static_pointer_cast<cppmicroservices::ServiceFactory>(factorySvc));

        // grab the dependent service, make sure it bound succesfully
        auto dependentSvcRef = context.GetServiceReference<test::FactoryServiceDependent>();
        auto dependentSvc = context.GetService<test::FactoryServiceDependent>(dependentSvcRef);
        ASSERT_TRUE(dependentSvc && dependentSvc->didBind());

        // verify that two calls to getServiceFromReference will return the same service, with the right interfaceid
        ASSERT_NO_THROW(ASSERT_EQ(cppmicroservices::ServiceReferenceFromService(
                                      context.GetService<test::FactoryCreatedService>(
                                          context.GetServiceReference<test::FactoryCreatedService>()))
                                      .GetInterfaceId(),
                                  "test::FactoryCreatedService"));
        ASSERT_NO_THROW(
            ASSERT_EQ(cppmicroservices::ServiceReferenceFromService(context.GetService<test::FactoryCreatedService>(
                          context.GetServiceReference<test::FactoryCreatedService>())),
                      cppmicroservices::ServiceReferenceFromService(context.GetService<test::FactoryCreatedService>(
                          context.GetServiceReference<test::FactoryCreatedService>()))));

        /**
         * Verify that a service's configuration can be updated from within a 'Modified' callback without
         * deadlocking the framework trying to get the lock in configNotifier
         */
        TEST_F(IntegrationTestFixture, testUpdateConfigFromWithinModifiedCallback)
        {
            auto const& param = std::make_shared<AsyncWorkServiceThreadPool>(10);
            auto context = framework.GetBundleContext();

            // ASYNCWORKSERVICE
            auto reg = context.RegisterService<cppmicroservices::async::AsyncWorkService>(param);

            // CA Service
            auto CARef = context.GetServiceReference<cppmicroservices::service::cm::ConfigurationAdmin>();
            auto configAdminService = context.GetService<cppmicroservices::service::cm::ConfigurationAdmin>(CARef);
            ASSERT_TRUE(configAdminService) << "GetService failed for ConfigurationAdmin";

            // Start bundle
            std::string componentName = "sample::ServiceComponentCA10";

            auto testBundle = ::test::InstallAndStartBundle(context, "TestBundleDSCA10");
            ::test::InstallAndStartBundle(context, "TestBundleDSCA03");

            auto configObject = configAdminService->GetConfiguration(componentName);

            auto props = configObject->GetProperties();
            auto configObject2 = configAdminService->GetConfiguration("sample::ServiceComponentCA02");

            props["uniqueProp"] = std::string("UNIQUE");
            auto fut = configObject->Update(props);
            fut.get();
            fut = configObject2->Update(props);
            fut.get();
            props["configID"] = std::string("sample::ServiceComponentCA03");
            fut = configObject->Update(props);
            fut.get();

            // GetService to make component active
            auto serviceRef = context.GetServiceReference<test::CAInterface>();
            auto service = context.GetService<test::CAInterface>(serviceRef);
            ASSERT_TRUE(service) << "GetService failed for CAInterface";

            ASSERT_NO_THROW(reg.Unregister());
        }

        TEST_F(IntegrationTestFixture, TestConcurrentBundleInstalls)
        {
            auto const& param = std::make_shared<AsyncWorkServiceThreadPool>(20);
            auto ctx = framework.GetBundleContext();
            auto reg = ctx.RegisterService<cppmicroservices::async::AsyncWorkService>(param);

            auto paths = ::test::GetAllTestBundleLocations();

            constexpr int numThreads = 18;
            Barrier b(numThreads);
            std::vector<std::future<void>> futures;
            std::atomic<bool> failed = false;

            for (int i = 0; i < numThreads; ++i)
            {
                futures.emplace_back(std::async(std::launch::async,
                                                [&]()
                                                {
                                                    b.Wait();
                                                    auto threadCtx = framework.GetBundleContext();

                                                    std::map<std::string, std::string> installedBundles;

                                                    for (auto const& path : paths)
                                                    {
                                                        std::vector<cppmicroservices::Bundle> bundles;
                                                        try
                                                        {
                                                            bundles = threadCtx.InstallBundles(path);
                                                        }
                                                        catch (...)
                                                        {
                                                            // ignore malformed bundles
                                                            continue;
                                                        }
                                                        for (auto const& bundle : bundles)
                                                        {
                                                            std::string name = bundle.GetSymbolicName();
                                                            auto const [preexistingBundle, newBundleWasInserted]
                                                                = installedBundles.try_emplace(name, path);
                                                            if (!newBundleWasInserted)
                                                            {
                                                                std::cout << "Bundle '" << name
                                                                          << "' already installed from '"
                                                                          << preexistingBundle->second
                                                                          << "'. New install attempt from '" << path
                                                                          << "'." << std::endl;
                                                                failed.store(true);
                                                            }
                                                        }
                                                    }
                                                }));
            }

            // Wait for all threads to complete
            for (auto& fut : futures)
            {
                fut.get();
            }

            ASSERT_FALSE(failed.load());
        }

        TEST_F(IntegrationTestFixture, VerifyNoDuplicateDeactivate)
        {
            auto ctx = framework.GetBundleContext();
            auto reg = ctx.RegisterService<cppmicroservices::async::AsyncWorkService>(
                std::make_shared<AsyncWorkServiceThreadPool>(20));

            // install and start referencing service
            auto path = "DSGraph04";
            auto bundle = ::test::InstallAndStartBundle(ctx, path);

            // create referenced service instance
            auto initialRefdSvcReg = ctx.RegisterService<test::DSGraph05>(std::make_shared<test::basicDS5>());

            auto sRef = ctx.GetServiceReference<test::DSGraph04>();
            ASSERT_TRUE(sRef);
            auto svc = ctx.GetService<test::DSGraph04>(sRef);

            ASSERT_EQ(svc->Description(), "DSGraph04");

            constexpr int numThreads = 2;
            Barrier beforeStart(numThreads);
            std::vector<std::future<void>> futures;

            futures.emplace_back(std::async(std::launch::async,
                                            [&]()
                                            {
                                                beforeStart.Wait();
                                                initialRefdSvcReg.Unregister();
                                            }));

            futures.emplace_back(std::async(
                std::launch::async,
                [&]()
                {
                    beforeStart.Wait();

                    auto props = cppmicroservices::ServiceProperties({
                        { std::string("component.name"), std::string("test1") }
                    });

                    auto reg = ctx.RegisterService<test::DSGraph05>(std::make_shared<test::basicDS5>(), props);
                }));

            for (auto& fut : futures)
            {
                fut.get();
            }

            sRef = ctx.GetServiceReference<test::DSGraph04>();
            ASSERT_TRUE(sRef);
            svc = ctx.GetService<test::DSGraph04>(sRef);
            ASSERT_EQ(svc->Description(), "DSGraph04");
        }

        TEST_F(IntegrationTestFixture, TestGetComponentDescriptionDTOsWithConcurrentBundleStops)
        {
            auto ctx = framework.GetBundleContext();

            // Install and start multiple test bundles
            std::vector<cppmicroservices::Bundle> installedBundles;
            std::vector<std::string> bundlesToInstall
                = { "DSGraph01", "DSGraph02", "DSGraph03", "DSGraph04", "DSGraph05", "DSGraph06", "DSGraph07" };

            for (auto const& bundleName : bundlesToInstall)
            {
                auto bundle = ::test::InstallAndStartBundle(ctx, bundleName);
                ASSERT_TRUE(bundle) << "Failed to install bundle: " << bundleName;
                installedBundles.push_back(bundle);
            }

            // Verify all bundles are active
            for (auto const& bundle : installedBundles)
            {
                ASSERT_EQ(bundle.GetState(), cppmicroservices::Bundle::STATE_ACTIVE);
            }

            constexpr int numThreads = 10;
            std::atomic<bool> stopFlag { false };
            std::atomic<int> exceptionCount { 0 };
            std::atomic<int> successfulCallCount { 0 };
            std::atomic<int> bundlesStoppedCount { 0 };

            Barrier startBarrier(numThreads + 1);
            std::vector<std::future<void>> futures;

            // Threads that continuously call GetComponentDescriptionDTOs
            for (int i = 0; i < numThreads; ++i)
            {
                futures.emplace_back(std::async(
                    std::launch::async,
                    [&]()
                    {
                        startBarrier.Wait();

                        while (!stopFlag.load())
                        {
                            try
                            {
                                // Get DTOs for all bundles
                                auto dtos = dsRuntimeService->GetComponentDescriptionDTOs();
                                successfulCallCount++;
                            }
                            catch (std::exception const& e)
                            {
                                exceptionCount++;
                                std::cerr << "Unexpected exception in GetComponentDescriptionDTOs: " << e.what()
                                          << std::endl;
                            }
                            catch (...)
                            {
                                exceptionCount++;
                                std::cerr << "Unexpected unknown exception in GetComponentDescriptionDTOs" << std::endl;
                            }
                            std::this_thread::yield();
                        }
                    }));
            }

            // Thread that stops bundles continuously
            futures.emplace_back(std::async(std::launch::async,
                                            [&]()
                                            {
                                                startBarrier.Wait();

                                                for (auto& bundle : installedBundles)
                                                {
                                                    if (stopFlag.load())
                                                    {
                                                        break;
                                                    }

                                                    try
                                                    {
                                                        bundle.Stop();
                                                        bundlesStoppedCount++;
                                                    }
                                                    catch (...)
                                                    {
                                                        // Bundle might already be stopped, that's ok
                                                    }
                                                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                                                }
                                            }));

            // Wait until all bundles are stopped or timeout
            auto startTime = std::chrono::steady_clock::now();
            while (bundlesStoppedCount.load() < static_cast<int>(installedBundles.size()))
            {
                if (std::chrono::steady_clock::now() - startTime > std::chrono::seconds(5))
                {
                    break; // Timeout after 5 seconds
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            // Let a few more calls happen after bundles are stopped
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            stopFlag.store(true);

            // Wait for all threads to complete
            for (auto& fut : futures)
            {
                fut.get();
            }

            // Verify no exceptions were thrown
            EXPECT_EQ(exceptionCount.load(), 0)
                << "GetComponentDescriptionDTOs threw " << exceptionCount.load() << " exceptions";

            // Verify we made successful calls
            EXPECT_GT(successfulCallCount.load(), 0) << "No successful calls to GetComponentDescriptionDTOs were made";

            // Final verification - call should still work even with stopped bundles
            EXPECT_NO_THROW({
                auto finalDtos = dsRuntimeService->GetComponentDescriptionDTOs();
                // Some DTOs might still be returned for bundles that weren't stopped
                // or for bundles in RESOLVED state
            });
        }
    } // namespace test