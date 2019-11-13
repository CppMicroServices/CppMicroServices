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

#include "gtest/gtest.h"
#include "TestFixture.hpp"
#include "TestUtils.hpp"
#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/ServiceEvent.h"

namespace sc  = cppmicroservices::service::component;
namespace scr = cppmicroservices::service::component::runtime;

namespace test
{
/**
 * Verify a component that implements Activate & Deactivate methods receives
 * the callbacks
 */
TEST_F(tServiceComponent, testLifeCycleHooks) //DS_TOI_10
{
  auto ctxt = framework.GetBundleContext();
  auto testBundle = StartTestBundle("TestBundleDSTOI10");

  // use Service Registry to validate the component is available
  auto sRef = ctxt.GetServiceReference<test::LifeCycleValidation>();
  ASSERT_TRUE(static_cast<bool>(sRef));
  auto service = ctxt.GetService<test::LifeCycleValidation>(sRef);
  ASSERT_NE(service, nullptr);
  EXPECT_TRUE(service->IsActivated()) << "service component instance must have received the Activate callback";

  // Use DS runtime service to validate the component state
  auto compDesc = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent10");
  auto compConfigs = dsRuntimeService->GetComponentConfigurationDTOs(compDesc);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::ACTIVE);

  // Disable the component which should remove the corresponding configuration
  auto fut = dsRuntimeService->DisableComponent(compDesc);
  EXPECT_NO_THROW(fut.get());
  EXPECT_TRUE(service->IsDeactivated()) << "service component instance must have received the Deactivate callback";
  compConfigs = dsRuntimeService->GetComponentConfigurationDTOs(compDesc);
  EXPECT_EQ(compConfigs.size(), 0ul) << "No configurations must exist after the component is disbled";

  // stop the bundle
  testBundle.Stop();
  compDesc = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent10");
  EXPECT_EQ(compDesc.name, "") << "Component must not be found after bundle is stopped";
}

/**
 * verify state progressions for a component which throws from the lifecycle callbacks
 */
TEST_F(tServiceComponent, testThrowingLifeCycleHooks) //DS_TOI_9
{
  auto testBundle = StartTestBundle("TestBundleDSTOI9");
  auto ctxt = framework.GetBundleContext();
  auto sRef = ctxt.GetServiceReference<test::LifeCycleValidation>();
  ASSERT_TRUE(static_cast<bool>(sRef));
  auto service = ctxt.GetService<test::LifeCycleValidation>(sRef);
  EXPECT_EQ(service, nullptr) << "Service object must be nullptr since the service component should have thrown an exception when activated";
  auto compDesc = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent9");
  auto compConfigs = dsRuntimeService->GetComponentConfigurationDTOs(compDesc);
  EXPECT_EQ(compConfigs.size(), 1ul) << "One default config expected";
  EXPECT_EQ(compConfigs.at(0).state, scr::dto::ComponentState::SATISFIED) << "state must be SATISFIED, and never progresses to ACTIVE";
}

template <typename Task, typename Predicate>
bool RepeatTaskUntilOrTimeout(Task&& t, Predicate&& p)
{
  auto startTime = std::chrono::system_clock::now();
  do
  {
    t();
    std::chrono::duration<double> duration = std::chrono::system_clock::now() - startTime;
    if(duration > std::chrono::milliseconds(30000))
    {
      return false;
    }
  } while(!p());
  return true;
}

/**
 * verify state progressions for a immediate component
 * UNSATISFIED_REFERENCE -> ACTIVE -> UNSATISFIED_REFERENCE
 */
TEST_F(tServiceComponent, testImmediateComponent_LifeCycle) // DS_TOI_51
{
  auto testBundle = StartTestBundle("TestBundleDSTOI5");
  auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent5");
  auto compConfigDTOs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigDTOs.size(), 1ul);
  EXPECT_EQ(compConfigDTOs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE);
  auto ctxt = framework.GetBundleContext();
  auto sRef = ctxt.GetServiceReference<test::Interface2>();
  EXPECT_FALSE(static_cast<bool>(sRef)) << "Service must not be available before it's dependency";
  auto depBundle = StartTestBundle("TestBundleDSTOI1");

  // wait for the asynchronous task to take effect
  auto result = RepeatTaskUntilOrTimeout([&compConfigDTOs, service = this->dsRuntimeService, &compDescDTO]() {
                                           compConfigDTOs = service->GetComponentConfigurationDTOs(compDescDTO);
                                         },
    [&compConfigDTOs]()->bool {
      return compConfigDTOs.at(0).state == scr::dto::ComponentState::ACTIVE;
    });

  ASSERT_TRUE(result) << "Timed out waiting for state to change to ACTIVE after the dependency became available";
  auto sRef1 = ctxt.GetServiceReference<test::Interface2>();
  EXPECT_TRUE(static_cast<bool>(sRef1)) << "Service must be available after it's dependency is available";
  auto service = ctxt.GetService<test::Interface2>(sRef1);

  ASSERT_NE(service, nullptr);
  EXPECT_NO_THROW(service->ExtendedDescription()) << "Throws if the dependency could not be found";
  depBundle.Stop();
  result = RepeatTaskUntilOrTimeout([&compConfigDTOs, service = this->dsRuntimeService, &compDescDTO]() {
                                      compConfigDTOs = service->GetComponentConfigurationDTOs(compDescDTO);
                                    },
    [&compConfigDTOs]()->bool {
      return compConfigDTOs.at(0).state == scr::dto::ComponentState::UNSATISFIED_REFERENCE;
    });
  ASSERT_TRUE(result) << "Timed out waiting for state to change to UNSATISFIED_REFERENCE after the dependency was removed";
  auto sRef2 = ctxt.GetServiceReference<test::Interface2>();
  EXPECT_FALSE(static_cast<bool>(sRef2)) << "Service must not be available after it's dependency is removed";
}

/**
 * verify state progressions for a immediate component
 * UNSATISFIED_REFERENCE -> ACTIVE -> UNSATISFIED_REFERENCE
 */
TEST_F(tServiceComponent, testImmediateComponent_LifeCycle_Dynamic) // DS_TOI_51
{
  auto testBundle = StartTestBundle("TestBundleDSTOI7");
  auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent7");
  auto compConfigDTOs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigDTOs.size(), 1ul);
  EXPECT_EQ(compConfigDTOs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE);
  auto ctxt = framework.GetBundleContext();
  auto sRef = ctxt.GetServiceReference<test::Interface2>();
  EXPECT_FALSE(static_cast<bool>(sRef)) << "Service must not be available before it's dependency";
  auto depBundle = StartTestBundle("TestBundleDSTOI1");

  // wait for the asynchronous task to take effect
  auto result = RepeatTaskUntilOrTimeout([&compConfigDTOs, service = this->dsRuntimeService, &compDescDTO]() {
                                           compConfigDTOs = service->GetComponentConfigurationDTOs(compDescDTO);
                                         },
    [&compConfigDTOs]()->bool {
      return compConfigDTOs.at(0).state == scr::dto::ComponentState::ACTIVE;
    });

  ASSERT_TRUE(result) << "Timed out waiting for state to change to ACTIVE after the dependency became available";
  auto sRef1 = ctxt.GetServiceReference<test::Interface2>();
  EXPECT_TRUE(static_cast<bool>(sRef1)) << "Service must be available after it's dependency is available";
  auto service = ctxt.GetService<test::Interface2>(sRef1);

  ASSERT_NE(service, nullptr);
  EXPECT_NO_THROW(service->ExtendedDescription()) << "Throws if the dependency could not be found";
  depBundle.Stop();
  result = RepeatTaskUntilOrTimeout([&compConfigDTOs, service = this->dsRuntimeService, &compDescDTO]() {
                                      compConfigDTOs = service->GetComponentConfigurationDTOs(compDescDTO);
                                    },
    [&compConfigDTOs]()->bool {
      return compConfigDTOs.at(0).state == scr::dto::ComponentState::UNSATISFIED_REFERENCE;
    });
  ASSERT_TRUE(result) << "Timed out waiting for state to change to UNSATISFIED_REFERENCE after the dependency was removed";
  auto sRef2 = ctxt.GetServiceReference<test::Interface2>();
  EXPECT_FALSE(static_cast<bool>(sRef2)) << "Service must not be available after it's dependency is removed";
}

/**
 * verify state progressions for a delayed component
 * UNSATISFIED_REFERENCE -> SATISFIED -> ACTIVE -> UNSATISFIED_REFERENCE
 */
TEST_F(tServiceComponent, testDelayedComponent_LifeCycle) //DS_TOI_52
{
  //
  auto testBundle = StartTestBundle("TestBundleDSTOI6");
  auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent6");
  auto compConfigDTOs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigDTOs.size(), 1ul);
  EXPECT_EQ(compConfigDTOs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE);
  auto ctxt = framework.GetBundleContext();
  auto sRef = ctxt.GetServiceReference<test::Interface2>();
  EXPECT_FALSE(static_cast<bool>(sRef)) << "Service must not be available before it's dependency";
    

  std::mutex mtx, mtx1;
  std::condition_variable cv, cv1;
  bool serviceRegistered = false;
  bool serviceUnregistered = false;
  // add a listener
  auto token = ctxt.AddServiceListener([&](const cppmicroservices::ServiceEvent& evt) {
//      std::cout << evt << std::endl;
//      std::cout << GetServiceInterface(evt.GetServiceReference()) << ", " << us_service_interface_iid<test::Interface2>() << std::endl;
                                         if (evt.GetType() == cppmicroservices::ServiceEvent::SERVICE_REGISTERED &&
                                             evt.GetServiceReference().IsConvertibleTo(us_service_interface_iid<test::Interface2>()))
                                         {
                                           std::lock_guard<std::mutex> lk(mtx);
                                           serviceRegistered = true;
                                           cv.notify_all();
                                         }
                                       });
  auto depBundle = StartTestBundle("TestBundleDSTOI1");
  {
    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk, [&serviceRegistered]() {
                  return serviceRegistered == true;
                });
    ctxt.RemoveListener(std::move(token));
  }

  compConfigDTOs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigDTOs.at(0).state, scr::dto::ComponentState::SATISFIED);
  auto sRef1 = ctxt.GetServiceReference<test::Interface2>();
  ASSERT_TRUE(static_cast<bool>(sRef1)) << "Service must be available after it's dependency is available";
  auto service = ctxt.GetService<test::Interface2>(sRef1);
  ASSERT_NE(service, nullptr);
  compConfigDTOs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigDTOs.at(0).state, scr::dto::ComponentState::ACTIVE) << "State must be ACTIVE after call to GetService";
  EXPECT_NO_THROW(service->ExtendedDescription()) << "Throws if the dependency could not be found";
  auto token1 = ctxt.AddServiceListener([&](const cppmicroservices::ServiceEvent& evt) {
                                          //std::cout << evt << std::endl;
                                          auto sRef = evt.GetServiceReference();
                                          if (evt.GetType() == cppmicroservices::ServiceEvent::SERVICE_UNREGISTERING &&
                                              test::GetServiceId(sRef) == test::GetServiceId(sRef1))
                                          {
                                            std::lock_guard<std::mutex> lk(mtx1);
                                            serviceUnregistered = true;
                                            cv1.notify_all();
                                          }
                                        });
    
  depBundle.Stop();
  {
    std::unique_lock<std::mutex> lk(mtx1);
    cv1.wait(lk, [&serviceUnregistered]() {
                   return serviceUnregistered == true;
                 });
    ctxt.RemoveListener(std::move(token1));
  }

  compConfigDTOs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigDTOs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE);
  auto sRef2 = ctxt.GetServiceReference<test::Interface2>();
  EXPECT_FALSE(static_cast<bool>(sRef2)) << "Service must not be available after it's dependency is removed";
}
}
