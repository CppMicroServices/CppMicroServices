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

#include <future>
#include <random>
#include <thread>
#include <tuple>

#include "../../src/ConfigurationListenerImpl.hpp"
#include "../../src/SCRAsyncWorkService.hpp"
#include "../../src/SCRExtensionRegistry.hpp"
#include "../../src/SCRLogger.hpp"
#include "../../src/manager/BundleLoader.hpp"
#include "../../src/manager/BundleOrPrototypeComponentConfiguration.hpp"
#include "../../src/manager/ComponentConfigurationImpl.hpp"
#include "../../src/manager/ReferenceManager.hpp"
#include "../../src/manager/SingletonComponentConfiguration.hpp"
#include "../../src/manager/states/CCActiveState.hpp"
#include "../../src/manager/states/CCRegisteredState.hpp"
#include "../../src/manager/states/CCUnsatisfiedReferenceState.hpp"
#include "ConcurrencyTestUtil.hpp"
#include "Mocks.hpp"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/ServiceInterface.h"

#include "../TestUtils.hpp"
#include <TestInterfaces/Interfaces.hpp>

using cppmicroservices::service::component::ComponentContext;

namespace cppmicroservices
{
    namespace scrimpl
    {

        class MockCompConfigImplWithOverridenGetMetadata : public MockComponentConfigurationImpl
        {
          public:
            MockCompConfigImplWithOverridenGetMetadata(std::shared_ptr<metadata::ComponentMetadata const> metadata,
                                                       Bundle const& bundle,
                                                       std::shared_ptr<ComponentRegistry> registry,
                                                       std::shared_ptr<cppmicroservices::logservice::LogService> logger,
                                                       std::shared_ptr<ConfigurationNotifier> notifier)
                : MockComponentConfigurationImpl(metadata, bundle, registry, logger, notifier)
            {
            }
            virtual ~MockCompConfigImplWithOverridenGetMetadata() = default;
            MOCK_CONST_METHOD0(GetMetadata, std::shared_ptr<metadata::ComponentMetadata const>());
        };

        class ComponentFactoryImplTest : public ::testing::Test
        {
          protected:
            ComponentFactoryImplTest() : framework(cppmicroservices::FrameworkFactory().NewFramework()) {}

            virtual ~ComponentFactoryImplTest() = default;

            virtual void
            SetUp()
            {
                framework.Start();
                mockMetadata = std::make_shared<metadata::ComponentMetadata>();
                mockRegistry = std::make_shared<MockComponentRegistry>();
                fakeLogger = std::make_shared<FakeLogger>();
                logger = std::make_shared<SCRLogger>(GetFramework().GetBundleContext());
                asyncWorkService = std::make_shared<cppmicroservices::scrimpl::SCRAsyncWorkService>(
                    GetFramework().GetBundleContext(),
                    logger);
                extRegistry = std::make_shared<SCRExtensionRegistry>(logger);
                notifier = std::make_shared<ConfigurationNotifier>(GetFramework().GetBundleContext(),
                                                                   fakeLogger,
                                                                   asyncWorkService,
                                                                   extRegistry);

                fakeCompConfig = std::make_shared<MockCompConfigImplWithOverridenGetMetadata>(mockMetadata,
                                                                                              GetFramework(),
                                                                                              mockRegistry,
                                                                                              fakeLogger,
                                                                                              notifier);
                EXPECT_CALL(*fakeCompConfig, GetMetadata()).WillRepeatedly(testing::Return(mockMetadata));
            }

            virtual void
            TearDown()
            {
                framework.Stop();
                framework.WaitForStop(std::chrono::milliseconds::zero());
            }

            cppmicroservices::Framework&
            GetFramework()
            {
                return framework;
            }

          protected:
            cppmicroservices::Framework framework;
            cppmicroservices::BundleContext context;
            std::shared_ptr<metadata::ComponentMetadata> mockMetadata;
            std::shared_ptr<MockComponentRegistry> mockRegistry;
            std::shared_ptr<FakeLogger> fakeLogger;
            std::shared_ptr<SCRLogger> logger;
            std::shared_ptr<cppmicroservices::scrimpl::SCRAsyncWorkService> asyncWorkService;
            std::shared_ptr<SCRExtensionRegistry> extRegistry;
            std::shared_ptr<ConfigurationNotifier> notifier;
            std::shared_ptr<MockCompConfigImplWithOverridenGetMetadata> fakeCompConfig;
        };

        TEST_F(ComponentFactoryImplTest, verifyNameCreation)
        {
            std::string compName = "someName";
            std::string id = "123";
            mockMetadata->name = compName;

            EXPECT_CALL(*mockRegistry,
                        AddComponentManager(::testing::Truly([compName, id](std::shared_ptr<ComponentManager> manager)
                                                             { return manager->GetName() == compName + "_" + id; })))
                .WillRepeatedly(::testing::Return(true));

            cppmicroservices::AnyMap configData(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
            configData["bar"] = std::string { "baz" };

            auto compFact = std::make_shared<ComponentFactoryImpl>(GetFramework().GetBundleContext(),
                                                                   fakeLogger,
                                                                   asyncWorkService,
                                                                   extRegistry);

            std::shared_ptr<ComponentConfigurationImpl> compConfigImpl = fakeCompConfig;

            compFact->CreateFactoryComponent(id, compConfigImpl, configData);
        }

        TEST_F(ComponentFactoryImplTest, verifyDynamicTargetInConfig)
        {
            std::string compName = "someName";
            std::string id = "123";
            mockMetadata->name = compName;

            std::string refName = "someRef";
            std::string targetKey = refName + ".target";
            std::string expectedTargetValue = "(someValidLdap=someValidValue)";

            cppmicroservices::scrimpl::metadata::ReferenceMetadata refMetadata;
            refMetadata.name = refName;
            refMetadata.target = "(someOtherTarget=ToBeOverwritten)";
            mockMetadata->refsMetadata.push_back(refMetadata);

            EXPECT_CALL(*mockRegistry,
                        AddComponentManager(::testing::Truly(
                            [compName, id, refName, expectedTargetValue](std::shared_ptr<ComponentManager> manager)
                            {
                                // Implement your logic to check the property
                                return manager->GetName() == compName + "_" + id
                                       && manager->GetMetadata()->refsMetadata[0].name == refName
                                       && manager->GetMetadata()->refsMetadata[0].target == expectedTargetValue;
                            })))
                .WillRepeatedly(::testing::Return(true));

            cppmicroservices::AnyMap configData(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
            configData["bar"] = std::string { "baz" };
            configData[targetKey] = expectedTargetValue;

            auto compFact = std::make_shared<ComponentFactoryImpl>(GetFramework().GetBundleContext(),
                                                                   fakeLogger,
                                                                   asyncWorkService,
                                                                   extRegistry);

            std::shared_ptr<ComponentConfigurationImpl> compConfigImpl = fakeCompConfig;

            compFact->CreateFactoryComponent(id, compConfigImpl, configData);
        }

        TEST_F(ComponentFactoryImplTest, verifyDynamicTargetUsingPlaceholder)
        {
            std::string compName = "someName";
            std::string id = "123";
            mockMetadata->name = compName;

            std::string refName = "someRef";
            std::string expectedTargetValue = "(someValidLdap=someValidValue)";

            cppmicroservices::scrimpl::metadata::ReferenceMetadata refMetadata;
            refMetadata.name = refName;
            refMetadata.target = "(someValidLdap={{keyToConfig}})";
            mockMetadata->refsMetadata.push_back(refMetadata);

            EXPECT_CALL(*mockRegistry,
                        AddComponentManager(::testing::Truly(
                            [compName, id, refName, expectedTargetValue](std::shared_ptr<ComponentManager> manager)
                            {
                                // Implement your logic to check the property
                                return manager->GetName() == compName + "_" + id
                                       && manager->GetMetadata()->refsMetadata[0].name == refName
                                       && manager->GetMetadata()->refsMetadata[0].target == expectedTargetValue;
                            })))
                .WillRepeatedly(::testing::Return(true));

            cppmicroservices::AnyMap configData(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
            configData["bar"] = std::string { "baz" };
            configData["keyToConfig"] = std::string { "someValidValue" };

            auto compFact = std::make_shared<ComponentFactoryImpl>(GetFramework().GetBundleContext(),
                                                                   fakeLogger,
                                                                   asyncWorkService,
                                                                   extRegistry);

            std::shared_ptr<ComponentConfigurationImpl> compConfigImpl = fakeCompConfig;

            compFact->CreateFactoryComponent(id, compConfigImpl, configData);
        }

        TEST_F(ComponentFactoryImplTest, verify2DynamicTargetUsingPlaceholder)
        {
            std::string compName = "someName";
            std::string id = "123";
            mockMetadata->name = compName;

            std::string refName = "someRef";
            std::string expectedTargetValue = "(&(whatIf=Woah)(IWanted=What))";

            cppmicroservices::scrimpl::metadata::ReferenceMetadata refMetadata;
            refMetadata.name = refName;
            refMetadata.target = "(&(whatIf={{TwoKeys}})(IWanted={{ToTheConfig}}))";
            mockMetadata->refsMetadata.push_back(refMetadata);

            EXPECT_CALL(*mockRegistry,
                        AddComponentManager(::testing::Truly(
                            [compName, id, refName, expectedTargetValue](std::shared_ptr<ComponentManager> manager)
                            {
                                // Implement your logic to check the property
                                return manager->GetName() == compName + "_" + id
                                       && manager->GetMetadata()->refsMetadata[0].name == refName
                                       && manager->GetMetadata()->refsMetadata[0].target == expectedTargetValue;
                            })))
                .WillRepeatedly(::testing::Return(true));

            cppmicroservices::AnyMap configData(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
            configData["bar"] = std::string { "baz" };
            configData["TwoKeys"] = std::string { "Woah" };
            configData["ToTheConfig"] = std::string { "What" };

            auto compFact = std::make_shared<ComponentFactoryImpl>(GetFramework().GetBundleContext(),
                                                                   fakeLogger,
                                                                   asyncWorkService,
                                                                   extRegistry);

            std::shared_ptr<ComponentConfigurationImpl> compConfigImpl = fakeCompConfig;

            compFact->CreateFactoryComponent(id, compConfigImpl, configData);
        }

        TEST_F(ComponentFactoryImplTest, prioritizeOSGITarget)
        {
            std::string compName = "someName";
            std::string id = "123";
            mockMetadata->name = compName;

            std::string refName = "someRef";
            std::string targetKey = refName + ".target";
            std::string expectedTargetValue = "(&(We=Prioritize)(OSGI=behavior))";

            cppmicroservices::scrimpl::metadata::ReferenceMetadata refMetadata;
            refMetadata.name = refName;
            refMetadata.target = "(&(whatIf={{TwoKeys}})(IWanted={{ToTheConfig}}))";
            mockMetadata->refsMetadata.push_back(refMetadata);

            EXPECT_CALL(*mockRegistry,
                        AddComponentManager(::testing::Truly(
                            [compName, id, refName, expectedTargetValue](std::shared_ptr<ComponentManager> manager)
                            {
                                // Implement your logic to check the property
                                return manager->GetName() == compName + "_" + id
                                       && manager->GetMetadata()->refsMetadata[0].name == refName
                                       && manager->GetMetadata()->refsMetadata[0].target == expectedTargetValue;
                            })))
                .WillRepeatedly(::testing::Return(true));

            cppmicroservices::AnyMap configData(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
            configData["bar"] = std::string { "baz" };
            configData[targetKey] = expectedTargetValue;
            configData["TwoKeys"] = std::string { "Woah" };
            configData["ToTheConfig"] = std::string { "What" };

            auto compFact = std::make_shared<ComponentFactoryImpl>(GetFramework().GetBundleContext(),
                                                                   fakeLogger,
                                                                   asyncWorkService,
                                                                   extRegistry);

            std::shared_ptr<ComponentConfigurationImpl> compConfigImpl = fakeCompConfig;

            compFact->CreateFactoryComponent(id, compConfigImpl, configData);
        }

        TEST_F(ComponentFactoryImplTest, failureModes)
        {
            std::string compName = "someName";
            std::string id = "123";
            mockMetadata->name = compName;

            std::string refName = "someRef";
            std::string targetKey = refName + ".target";

            cppmicroservices::scrimpl::metadata::ReferenceMetadata refMetadata;
            refMetadata.name = refName;
            refMetadata.target = "(&(whatIf={{TwoKeysOops}})(IWanted={{ToTheConfig}}))";
            mockMetadata->refsMetadata.push_back(refMetadata);

            cppmicroservices::AnyMap configData(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
            configData["bar"] = std::string { "baz" };
            configData["TwoKeys"] = std::string { "Woah" };
            configData["ToTheConfig"] = std::string { "What" };

            auto compFact = std::make_shared<ComponentFactoryImpl>(GetFramework().GetBundleContext(),
                                                                   fakeLogger,
                                                                   asyncWorkService,
                                                                   extRegistry);

            std::shared_ptr<ComponentConfigurationImpl> compConfigImpl = fakeCompConfig;

            EXPECT_THROW({ compFact->CreateFactoryComponent(id, compConfigImpl, configData); }, std::invalid_argument);
            // valid keys, but invalid ldap
            refMetadata.target = "(&(whatIf={{TwoKeys}})(IWanted={{ToTheConfig}))";
            EXPECT_THROW({ compFact->CreateFactoryComponent(id, compConfigImpl, configData); }, std::invalid_argument);
            // extra "{"
            refMetadata.target = "(&(whatIf={{TwoKeys}})(IWanted={{ToTheConfig}}{))";
            EXPECT_THROW({ compFact->CreateFactoryComponent(id, compConfigImpl, configData); }, std::invalid_argument);
        }
    } // namespace scrimpl
} // namespace cppmicroservices