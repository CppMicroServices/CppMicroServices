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
#include "../../src/SCRExtensionRegistry.hpp"
#include "../../src/manager/SingletonComponentConfiguration.hpp"
#include "../../src/manager/states/CCActiveState.hpp"
#include "../../src/manager/states/CCRegisteredState.hpp"
#include "ConcurrencyTestUtil.hpp"
#include "Mocks.hpp"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/logservice/LogService.hpp"

namespace cppmicroservices
{
    namespace scrimpl
    {

        class SingletonComponentConfigurationTest : public ::testing::Test
        {
          protected:
            SingletonComponentConfigurationTest() : framework(cppmicroservices::FrameworkFactory().NewFramework()) {}

            virtual ~SingletonComponentConfigurationTest() = default;

            virtual void
            SetUp()
            {
                framework.Start();
                auto mockMetadata = std::make_shared<metadata::ComponentMetadata>();
                auto mockRegistry = std::make_shared<MockComponentRegistry>();
                mockLogger = std::make_shared<MockLogger>();
                auto logger = std::make_shared<cppmicroservices::scrimpl::SCRLogger>(framework.GetBundleContext());
                auto asyncWorkService
                    = std::make_shared<cppmicroservices::scrimpl::SCRAsyncWorkService>(framework.GetBundleContext(),
                                                                                       logger);
                auto extRegistry = std::make_shared<SCRExtensionRegistry>(logger);
                auto notifier = std::make_shared<ConfigurationNotifier>(framework.GetBundleContext(),
                                                                        mockLogger,
                                                                        asyncWorkService,
                                                                        extRegistry); 
                obj = std::make_shared<SingletonComponentConfigurationImpl>(mockMetadata,
                                                                            framework,
                                                                            mockRegistry,
                                                                            mockLogger,
                                                                            notifier);
            }

            virtual void
            TearDown()
            {
                obj.reset();
                framework.Stop();
                framework.WaitForStop(std::chrono::milliseconds::zero());
            }

            cppmicroservices::Framework framework;
            std::shared_ptr<MockLogger> mockLogger;
            std::shared_ptr<SingletonComponentConfigurationImpl> obj;
        };

        ACTION(ModifiedMethodException) { throw "Component Instance Modified method exception"; }
        ACTION(ModifiedMethodExists) { return true; }
        TEST_F(SingletonComponentConfigurationTest, TestGetFactory) { EXPECT_NE(obj->GetFactory(), nullptr); }

        class MockComponentInstanceFactory
        {
          public:
            MOCK_METHOD0(CreateComponentInstance, ComponentInstance*());
            MOCK_METHOD1(DeleteComponentInstance, void(ComponentInstance*));
        };

        TEST_F(SingletonComponentConfigurationTest, TestCreateAndActivateComponentInstance)
        {
            // calling CreateAndActivateComponentInstance multiple times must return the same instance
            MockComponentInstanceFactory mockCompFactory;
            auto mockInstance = new MockComponentInstance();
            obj->SetState(std::make_shared<CCRegisteredState>());
            auto nullInstance = obj->CreateAndActivateComponentInstance(framework);
            EXPECT_EQ(nullInstance, nullptr) << "Return value must be nullptr when state is not ACTIVE";
            obj->SetState(std::make_shared<CCActiveState>());
            obj->SetComponentInstanceCreateDeleteMethods(
                std::bind(&MockComponentInstanceFactory::CreateComponentInstance, &mockCompFactory),
                std::bind(&MockComponentInstanceFactory::DeleteComponentInstance,
                          &mockCompFactory,
                          std::placeholders::_1));
            EXPECT_CALL(mockCompFactory, CreateComponentInstance())
                .Times(2)
                .WillOnce(testing::Throw(std::runtime_error("Some error in user code")))
                .WillOnce(testing::Return(mockInstance));
            EXPECT_CALL(mockCompFactory, DeleteComponentInstance(testing::_))
                .Times(1)
                .WillOnce(testing::Invoke([](ComponentInstance* obj) { delete obj; }));
            EXPECT_CALL(*mockInstance, CreateInstance(testing::_)).Times(1);
            EXPECT_CALL(*mockInstance, BindReferences(testing::_)).Times(1);
            EXPECT_CALL(*mockInstance, Activate()).Times(1);

            // set logging expectations
            auto ExceptionThrownByCreateComponentInstance
                = testing::AllOf(testing::HasSubstr("Exception received from user code while activating "),
                                 testing::HasSubstr("the component configuration"));
            EXPECT_CALL(*mockLogger,
                        Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                            ExceptionThrownByCreateComponentInstance,
                            testing::_))
                .Times(1);
            auto instance0 = obj->CreateAndActivateComponentInstance(framework);
            EXPECT_EQ(instance0, nullptr) << "Return value must be nullptr when an "
                                             "exception is thrown from user code";
            auto instance = obj->CreateAndActivateComponentInstance(framework);
            EXPECT_NE(instance, nullptr) << "Return value must be non-null in ACTIVE state";
            auto instance1 = obj->CreateAndActivateComponentInstance(framework);
            EXPECT_EQ(instance, instance1) << "Return values for repeated calls must be the same";

            // clean-up injected mock objects
            {
                auto instCtxtPair = obj->data.lock();
                instCtxtPair->first.reset();
                instCtxtPair->second.reset();
            }
        }

        TEST_F(SingletonComponentConfigurationTest, TestConcurrentCreateAndActivateComponentInstance)
        {
            // calling CreateAndActivateComponentInstance from multiple threads must return the same instance
            MockComponentInstanceFactory mockCompFactory;
            auto mockInstance = new MockComponentInstance();
            obj->SetState(std::make_shared<CCActiveState>());
            obj->SetComponentInstanceCreateDeleteMethods(
                std::bind(&MockComponentInstanceFactory::CreateComponentInstance, &mockCompFactory),
                std::bind(&MockComponentInstanceFactory::DeleteComponentInstance,
                          &mockCompFactory,
                          std::placeholders::_1));
            EXPECT_CALL(mockCompFactory, CreateComponentInstance()).Times(1).WillOnce(testing::Return(mockInstance));
            EXPECT_CALL(mockCompFactory, DeleteComponentInstance(testing::_))
                .Times(1)
                .WillOnce(testing::Invoke([](ComponentInstance* obj) { delete obj; }));
            EXPECT_CALL(*mockInstance, CreateInstance(testing::_)).Times(1);
            EXPECT_CALL(*mockInstance, BindReferences(testing::_)).Times(1);
            EXPECT_CALL(*mockInstance, Activate()).Times(1);

            std::function<std::shared_ptr<ComponentInstance>(void)> func
                = [&]() { return obj->CreateAndActivateComponentInstance(framework); };
            auto results = ConcurrentInvoke(func);
            if (!results.empty())
            {
                auto firstElement = results[0];
                EXPECT_TRUE(std::all_of(results.begin(),
                                        results.end(),
                                        [&](auto const& elem) { return elem == firstElement; }));
            }

            // clean up injected mock objects
            {
                auto instCtxtPair = obj->data.lock();
                instCtxtPair->first.reset();
                instCtxtPair->second.reset();
            }
        }

        TEST_F(SingletonComponentConfigurationTest, TestGetService)
        {
            MockComponentInstanceFactory mockCompFactory;
            auto mockInstance = std::make_shared<MockComponentInstance>();
            obj->SetState(std::make_shared<CCRegisteredState>());
            obj->SetComponentInstanceCreateDeleteMethods(
                std::bind(&MockComponentInstanceFactory::CreateComponentInstance, &mockCompFactory),
                std::bind(&MockComponentInstanceFactory::DeleteComponentInstance,
                          &mockCompFactory,
                          std::placeholders::_1));
            EXPECT_CALL(mockCompFactory, CreateComponentInstance())
                .Times(1)
                .WillOnce(testing::Return(mockInstance.get()));
            EXPECT_CALL(mockCompFactory, DeleteComponentInstance(testing::_)).Times(1);
            cppmicroservices::InterfaceMapPtr iMap;
            EXPECT_CALL(*mockInstance, CreateInstance(testing::_)).Times(1);
            EXPECT_CALL(*mockInstance, BindReferences(testing::_)).Times(1);
            EXPECT_CALL(*mockInstance, Activate()).Times(1);
            EXPECT_CALL(*mockInstance, GetInterfaceMap()).Times(1).WillOnce(testing::Return(iMap));
            auto service = obj->GetService(Bundle(), ServiceRegistrationU());
            EXPECT_EQ(obj->GetState()->GetValue(), ComponentState::ACTIVE);
            EXPECT_EQ(service, iMap);

            // clean up injected mock objects
            {
                auto instCtxtPair = obj->data.lock();
                instCtxtPair->first.reset();
                instCtxtPair->second.reset();
            }
        }
        /* This test verifies that if the Modified method of a component instance throws an
         * exception DS intercepts the exception and logs it.
         */
        TEST_F(SingletonComponentConfigurationTest, TestModifiedMethodExceptionLogging)
        {
            using cppmicroservices::logservice::SeverityLevel;

            auto mockCompContext = std::make_shared<MockComponentContextImpl>(obj);
            auto mockCompInstance = std::make_shared<MockComponentInstance>();
            obj->SetComponentInstancePair(InstanceContextPair(mockCompInstance, mockCompContext));
            EXPECT_NE(obj->GetComponentInstance(), nullptr);
            EXPECT_NE(obj->GetComponentContext(), nullptr);

            // set logging expectations
            auto ExceptionThrownByModifiedMethod
                = testing::AllOf(testing::HasSubstr("Exception received from user code while modifying "),
                                 testing::HasSubstr("component configuration"));
            EXPECT_CALL(*mockLogger, Log(SeverityLevel::LOG_ERROR, ExceptionThrownByModifiedMethod, testing::_))
                .Times(1);

            // When the mock Modified method is called it will throw the ModifiedMethodException
            EXPECT_CALL(*mockCompInstance, Modified()).Times(1).WillRepeatedly(ModifiedMethodException());

            // When the mock DoesModifiedMethodExist method is called it will return true;
            EXPECT_CALL(*mockCompInstance, DoesModifiedMethodExist()).Times(1).WillRepeatedly(ModifiedMethodExists());

            // Deactivate and Unbindreference will also be called. EXPECT_CALL added
            // to avoid GMOCK Warning.
            // When the mock Modified method is called it will throw the ModifiedMethodException
            EXPECT_CALL(*mockCompInstance, Deactivate()).Times(1);
            EXPECT_CALL(*mockCompInstance, UnbindReferences()).Times(1);

            // ModifyComponentInstanceProperties will call the mock Modified method
            // which will throw an exception. DS will catch the exception and log it.
            EXPECT_NO_THROW(obj->ModifyComponentInstanceProperties());
        }

        TEST_F(SingletonComponentConfigurationTest, TestDestroyComponentInstances)
        {
            auto mockCompContext = std::make_shared<MockComponentContextImpl>(obj);
            auto mockCompInstance = std::make_shared<MockComponentInstance>();
            obj->SetComponentInstancePair(InstanceContextPair(mockCompInstance, mockCompContext));
            EXPECT_CALL(*mockCompInstance, Deactivate()).Times(1);
            EXPECT_CALL(*mockCompInstance, UnbindReferences()).Times(1);
            EXPECT_NE(obj->GetComponentInstance(), nullptr);
            EXPECT_NE(obj->GetComponentContext(), nullptr);
            EXPECT_NO_THROW(obj->DestroyComponentInstances());
            EXPECT_EQ(obj->GetComponentInstance(), nullptr);
            EXPECT_EQ(obj->GetComponentContext(), nullptr);
            EXPECT_EQ(obj->GetState()->GetValue(), ComponentState::UNSATISFIED_REFERENCE);
        }

        TEST_F(SingletonComponentConfigurationTest, TestDestroyComponentInstances_DeactivateFailure)
        {
            auto mockCompContext = std::make_shared<MockComponentContextImpl>(obj);
            auto mockCompInstance = std::make_shared<MockComponentInstance>();
            obj->SetComponentInstancePair(InstanceContextPair(mockCompInstance, mockCompContext));
            std::string exceptionMsg("Deactivation failed with exception");

            // set logging expectations
            auto ExceptionThrownByDeactivateMethod
                = testing::AllOf(testing::HasSubstr("Exception received from user code while deactivating "),
                                 testing::HasSubstr("the component configuration"));
            EXPECT_CALL(*mockLogger,
                        Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                            ExceptionThrownByDeactivateMethod,
                            testing::_))
                .Times(1);
            EXPECT_CALL(*mockCompInstance, Deactivate())
                .Times(1)
                .WillOnce(testing::Throw(std::runtime_error(exceptionMsg)));
            EXPECT_NE(obj->GetComponentInstance(), nullptr);
            EXPECT_NE(obj->GetComponentContext(), nullptr);
            EXPECT_NO_THROW(obj->DestroyComponentInstances());
            EXPECT_EQ(obj->GetComponentInstance(), nullptr);
            EXPECT_EQ(obj->GetComponentContext(), nullptr);
            EXPECT_EQ(obj->GetState()->GetValue(), ComponentState::UNSATISFIED_REFERENCE);
        }
    } // namespace scrimpl
} // namespace cppmicroservices
