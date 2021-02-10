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

#if defined(US_PLATFORM_WINDOWS)
  #include <Windows.h>
  #include <strsafe.h>
  #include <psapi.h>
#else
  #include <iostream>
  #include <fstream>
#endif

#if defined(US_PLATFORM_LINUX)
  #include <linux/limits.h>
#endif

namespace sc  = cppmicroservices::service::component;
namespace scr = cppmicroservices::service::component::runtime;

namespace test
{
bool isErrored(const std::string functionName);
bool isBundleLoaded(const std::string bundleName);

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
  auto result = RepeatTaskUntilOrTimeout([&compConfigDTOs, service = this->dsRuntimeService, &compDescDTO]()
                                         {
                                             compConfigDTOs = service->GetComponentConfigurationDTOs(compDescDTO);
                                         }
                                         , [&compConfigDTOs]()->bool
                                           {
                                               return compConfigDTOs.at(0).state == scr::dto::ComponentState::ACTIVE;
                                           });

  ASSERT_TRUE(result) << "Timed out waiting for state to change to ACTIVE after the dependency became available";
  auto sRef1 = ctxt.GetServiceReference<test::Interface2>();
  EXPECT_TRUE(static_cast<bool>(sRef1)) << "Service must be available after it's dependency is available";
  auto service = ctxt.GetService<test::Interface2>(sRef1);

  ASSERT_NE(service, nullptr);
  EXPECT_NO_THROW(service->ExtendedDescription()) << "Throws if the dependency could not be found";
  depBundle.Stop();
  
  result = RepeatTaskUntilOrTimeout([&compConfigDTOs, service = this->dsRuntimeService, &compDescDTO]()
                                    {
                                      compConfigDTOs = service->GetComponentConfigurationDTOs(compDescDTO);
                                    }
                                    , [&compConfigDTOs]()->bool
                                      {
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
 * Verify state progressions for a delayed component
 * UNSATISFIED_REFERENCE -> SATISFIED -> ACTIVE -> UNSATISFIED_REFERENCE
 * Verify that the bundle is not loaded into the process before GetService is called.
 * Verify that the bundle is loaded into the process once GetService is called.
 */
TEST_F(tServiceComponent, testDelayedComponent_LifeCycle) //DS_TOI_52 //DS_TOI_6
{
  auto testBundle = StartTestBundle("TestBundleDSTOI6");
  auto compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, "sample::ServiceComponent6");
  auto compConfigDTOs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigDTOs.size(), 1ul);
  EXPECT_EQ(compConfigDTOs.at(0).state, scr::dto::ComponentState::UNSATISFIED_REFERENCE);
  auto ctxt = framework.GetBundleContext();
  auto sRef = ctxt.GetServiceReference<test::Interface2>();
  EXPECT_FALSE(static_cast<bool>(sRef)) << "Service must not be available before it's dependency";
  
  auto result = isBundleLoaded("TestBundleDSTOI6");
  EXPECT_FALSE(result) << "library must not be available";

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

  result = isBundleLoaded("TestBundleDSTOI6");
  EXPECT_FALSE(result) << "library must not be available";

  auto sRef1 = ctxt.GetServiceReference<test::Interface2>();
  ASSERT_TRUE(static_cast<bool>(sRef1)) << "Service must be available after it's dependency is available";
  auto service = ctxt.GetService<test::Interface2>(sRef1);
  ASSERT_NE(service, nullptr);
  compConfigDTOs = dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
  EXPECT_EQ(compConfigDTOs.at(0).state, scr::dto::ComponentState::ACTIVE) << "State must be ACTIVE after call to GetService";
  EXPECT_NO_THROW(service->ExtendedDescription()) << "Throws if the dependency could not be found";

  result = isBundleLoaded("TestBundleDSTOI6");
  EXPECT_TRUE(result) << "library must be available";

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

//To check if error occured during a last function call
bool isErrored(const std::string functionName)
{
    // Retrieve the system error message for the last-error code
    LPSTR lpDisplayBuf;
    DWORD dw = GetLastError();

    if (dw == 0)
        return false;

    std::size_t size = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpDisplayBuf,
        0, NULL);

    std::cerr << "\n" << functionName << " failed with error " << dw << ": " << lpDisplayBuf << std::endl;
    LocalFree(lpDisplayBuf);
    return true;
}

//Function to validate lazy loading of delayed component
bool isBundleLoaded(const std::string bundleName)
{
#if defined(US_PLATFORM_WINDOWS)

    HMODULE hMods[1024];
    DWORD cbNeeded;

    HANDLE hProcess = GetCurrentProcess();

    auto res = EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded);
    EXPECT_FALSE(isErrored("EnumProcessModules"));

    EXPECT_GT(sizeof(hMods), cbNeeded) << "Size of array is too small to hold all module handles";
    if ((sizeof(hMods) < cbNeeded))
    {
        return false;
    }

    TCHAR szModName[MAX_PATH];
    std::size_t found;

    for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
    {
        auto modulePathLength = GetModuleFileNameA(hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR));
        EXPECT_FALSE(isErrored("GetModuleFileNameA"));

        found = std::string(szModName).find(bundleName);
        if (found != std::string::npos)
        {
            CloseHandle(hProcess);
            EXPECT_FALSE(isErrored("CloseHandle"));
            return true;
        }
    }

    CloseHandle(hProcess);
    EXPECT_FALSE(isErrored("CloseHandle"));
    return false;
#else
    auto pid_t = getpid();
    std::string command("lsof -p " + std::to_string(pid_t));
    FILE* fd = popen(command.c_str(), "r");
    EXPECT_NE(fd, nullptr) << "popen failed";
    if (nullptr == fd)
    {
        return false;
    }

    std::size_t found;
    char buf[PATH_MAX];
    while (nullptr != fgets(buf, PATH_MAX, fd))
    {
        found = std::string(buf).find(bundleName);
        if (found != std::string::npos)
        {
            auto fc = pclose(fd);
            EXPECT_NE(fc, -1) << "pclose failed";
            return true;
        }
    }

    auto fc = pclose(fd);
    EXPECT_NE(fc, -1) << "pclose failed";
    return false;
#endif
}

}
