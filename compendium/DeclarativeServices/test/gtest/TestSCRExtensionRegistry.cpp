/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  =============================================================================*/

#include "../../src/SCRAsyncWorkService.hpp"
#include "../../src/SCRBundleExtension.hpp"
#include "../../src/SCRExtensionRegistry.hpp"
#include "../../src/manager/SingletonComponentConfiguration.hpp"
#include "../TestUtils.hpp"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include "gmock/gmock.h"
#include "Mocks.hpp"

namespace cppmicroservices
{
    namespace scrimpl
    {
        // The fixture for testing class SCRExtensionRegistry.
        class SCRExtensionRegistryTest : public ::testing::Test
        {
          protected:
            SCRExtensionRegistryTest() : framework(cppmicroservices::FrameworkFactory().NewFramework()) {}
            ~SCRExtensionRegistryTest() = default;

            void
            SetUp() override
            {
                framework.Start();
                fakeRegistry = std::make_shared<ComponentRegistry>();
                logger = std::make_shared<cppmicroservices::scrimpl::SCRLogger>(framework.GetBundleContext());
                asyncWorkService
                    = std::make_shared<cppmicroservices::scrimpl::SCRAsyncWorkService>(framework.GetBundleContext(),
                                                                                       logger);
                extRegistry = std::make_shared<SCRExtensionRegistry>(logger);
                notifier = std::make_shared<ConfigurationNotifier>(framework.GetBundleContext(),
                                                                   logger,
                                                                   asyncWorkService,
                                                                   extRegistry);
            }

            void
            TearDown() override
            {
                framework.Stop();
                framework.WaitForStop(std::chrono::milliseconds::zero());
            }

            cppmicroservices::Framework&
            GetFramework()
            {
                return framework;
            }

            void
            ConcurrentInvoke(std::vector<cppmicroservices::Bundle> allBundles, bool addOperation)
            {
                std::promise<void> go;
                std::shared_future<void> ready(go.get_future());
                int numCalls = allBundles.size();
                std::vector<std::promise<void>> readies(numCalls);
                std::vector<std::future<void>> bundle_result(numCalls);
                try
                {
                    if (addOperation)
                    {
                        for (int i = 0; i < numCalls; i++)
                        {

                            bundle_result[i]
                                = std::async(std::launch::async,
                                             [&readies, &ready, &allBundles, this, i]()
                                             {
                                                 readies[i].set_value();
                                                 ready.wait();
                                                 auto ba = std::make_shared<SCRBundleExtension>(allBundles[i],
                                                                                                this->fakeRegistry,
                                                                                                this->logger,
                                                                                                this->notifier);
                                                 extRegistry->Add(allBundles[i].GetBundleId(), ba);
                                             });
                        }
                    }
                    else // remove operation
                    {
                        for (int i = 0; i < numCalls; i++)
                        {
                            bundle_result[i] = std::async(std::launch::async,
                                                          [&readies, &ready, &allBundles, this, i]()
                                                          {
                                                              readies[i].set_value();
                                                              ready.wait();
                                                              this->extRegistry->Remove(allBundles[i].GetBundleId());
                                                          });
                        }
                    }

                    for (int i = 0; i < numCalls; i++)
                    {
                        readies[i].get_future().wait();
                    }
                    go.set_value();
                    for (int i = 0; i < numCalls; i++)
                    {
                        bundle_result[i].wait();
                    }
                }
                catch (std::exception const& e)
                {
                    EXPECT_TRUE(false) << "Error: exception received ... " << e.what() << std::endl;
                    go.set_value();
                    throw std::current_exception();
                }
            }
          protected:
            cppmicroservices::Framework framework;
            std::shared_ptr<ComponentRegistry> fakeRegistry;
            std::shared_ptr<cppmicroservices::scrimpl::SCRLogger> logger;
            std::shared_ptr<cppmicroservices::scrimpl::SCRAsyncWorkService> asyncWorkService;
            std::shared_ptr<SCRExtensionRegistry> extRegistry;
            std::shared_ptr<ConfigurationNotifier> notifier;
        };

        // Test constructor with invalid arguments
        TEST_F(SCRExtensionRegistryTest, CtorInvalidArgs)
        {
            EXPECT_THROW({ SCRExtensionRegistry bundleExt(nullptr); }, std::invalid_argument);
        }

        //Test constructor with valid arguments
        TEST_F(SCRExtensionRegistryTest, CtorWithValidArgs)
        {
            EXPECT_NO_THROW({ SCRExtensionRegistry bundleExt = SCRExtensionRegistry(logger); });
        }

        // Test Add, Find, Remove and Clear methods
        TEST_F(SCRExtensionRegistryTest, AddFindRemoveClear)
        {
            auto bundle = test::InstallAndStartBundle(GetFramework().GetBundleContext(), "TestBundleDSTOI1");
            ASSERT_TRUE(static_cast<bool>(bundle)) << "TestBundleDSTOI1 not installed.";
 
            // Test invalid arguments for Add method.
            EXPECT_THROW({ extRegistry->Add(bundle.GetBundleId(),nullptr); }, std::invalid_argument); 

            // Add a bundle extension to the SCRExtensionRegistry
            auto ba = std::make_shared<SCRBundleExtension>(bundle, fakeRegistry, logger, notifier);
            extRegistry->Add(bundle.GetBundleId(), ba);

            // Try to find the bundle extension just added.
            auto bundleExt = extRegistry->Find(bundle.GetBundleId());
            ASSERT_TRUE(bundleExt) << "SCRBundleExtension not found in Extension Registry";
 
            // Remove the bundle extension and make sure it's no longer there.
            extRegistry->Remove(bundle.GetBundleId());
            bundleExt = extRegistry->Find(bundle.GetBundleId());
            ASSERT_TRUE(!bundleExt) << "SCRBundleExtension found in Extension Registry. It should have been removed";
            
            // Add a bundle extension, clear the SCRExtensionRegistry and make sure the bundle extension is no longer there. 
            extRegistry->Add(bundle.GetBundleId(), ba);
            extRegistry->Clear();
            bundleExt = extRegistry->Find(bundle.GetBundleId());
            ASSERT_TRUE(!bundleExt) << "SCRBundleExtension found in Extension Registry. It should have been cleared";
 
            asyncWorkService->StopTracking();
            fakeRegistry->Clear();
        }
        // Test to test concurrent additions of bundle extensions to the SCRExtensionRegistry and
        // concurrent removals.
        TEST_F(SCRExtensionRegistryTest, VerifyConcurrentAddRemove)
        {
            int count = 4;
            test::InstallAndStartBundle(GetFramework().GetBundleContext(), "TestBundleDSTOI1");
            test::InstallAndStartBundle(GetFramework().GetBundleContext(), "TestBundleDSTOI2");
            test::InstallAndStartBundle(GetFramework().GetBundleContext(), "TestBundleDSTOI3");
            test::InstallAndStartBundle(GetFramework().GetBundleContext(), "TestBundleDSTOI5");
 
            auto bundleContext = GetFramework().GetBundleContext();
            auto allBundles = bundleContext.GetBundles();
            ASSERT_TRUE(allBundles.size() > count) << "All bundles not installed.";

            // Add a bundle extension object for each bundle in the allBundles vector to the 
            // extension registry
            ConcurrentInvoke(allBundles, true);           
            for (auto const& item : allBundles) {
                ASSERT_TRUE(extRegistry->Find(item.GetBundleId()))
                    << "bundle " << item.GetSymbolicName() << " not found.";
            }
 
            // Remove the bundle extension for all bundles in the allBundles vector from 
            // the extension registry.
            ConcurrentInvoke(allBundles, false);
            for (auto const& item : allBundles)
            {
                ASSERT_TRUE(!extRegistry->Find(item.GetBundleId()))
                    << "bundle " << item.GetSymbolicName() << " should have been removed.";
            }
            asyncWorkService->StopTracking();
            fakeRegistry->Clear();
  
        }

        //Mock SCRExtensionRegistry class used by the testException class
        class MockSCRExtensionRegistry : public cppmicroservices::scrimpl::SCRExtensionRegistry
        {
          public:
            MockSCRExtensionRegistry(std::shared_ptr<SCRLogger> const& logger)
                : cppmicroservices::scrimpl::SCRExtensionRegistry (logger)
            {
            }
            virtual ~MockSCRExtensionRegistry() = default;    
            MOCK_METHOD1(Find, std::shared_ptr<SCRBundleExtension>(long bundleId));
        }; 

        // Tests the CreateFactoryComponent method of the ConfigurationNotifier class. If the bundle extension 
        // cannot be found in the SCRExtensionRegistry when creating a factory component then an
        // exception will be logged.
        TEST_F(SCRExtensionRegistryTest, testException)
        {
            auto bundle = test::InstallAndStartBundle(GetFramework().GetBundleContext(), "TestBundleDSTOI1");
            ASSERT_TRUE(static_cast<bool>(bundle));
            auto mockExtRegistry = std::make_shared<MockSCRExtensionRegistry>(logger);
            auto mockLogger = std::make_shared<MockLogger>();
            auto notifier = std::make_shared<ConfigurationNotifier>(GetFramework().GetBundleContext(),
                                                                    mockLogger,
                                                                    asyncWorkService,
                                                                    mockExtRegistry);
  
            EXPECT_CALL(*mockExtRegistry, Find (bundle.GetBundleId())).Times(1).WillOnce(::testing::Return(nullptr));
            EXPECT_CALL(*(mockLogger.get()),
                        Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR, testing::_, testing::_))
                .Times(1);
            auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
 
 
            std::shared_ptr<ComponentConfigurationImpl> compConfigImpl
                = std::make_shared<SingletonComponentConfigurationImpl>(mockMetadata, bundle, fakeRegistry, logger, notifier);
            std::string pid { 123 };
            EXPECT_NO_THROW({ notifier->CreateFactoryComponent(pid, compConfigImpl); }); 
         }
    } // namespace scrimpl
} // namespace cppmicroservices
