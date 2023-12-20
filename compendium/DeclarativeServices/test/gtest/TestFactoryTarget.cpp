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

#include "TestFixture.hpp"
#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/LDAPFilter.h"
#include "cppmicroservices/LDAPProp.h"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "gtest/gtest.h"

namespace test
{

    class tFactoryTarget : public tGenericDSSuite
    {
      public:
        void
        SetUp() override
        {
            framework.Start();
            context = framework.GetBundleContext();

            ::test::InstallAndStartDS(context);
            auto sRef = context.GetServiceReference<scr::ServiceComponentRuntime>();
            ASSERT_TRUE(sRef);
            dsRuntimeService = context.GetService<scr::ServiceComponentRuntime>(sRef);
            ASSERT_TRUE(dsRuntimeService);

            ::test::InstallAndStartConfigAdmin(context);
            auto svcRef = context.GetServiceReference<cppmicroservices::service::cm::ConfigurationAdmin>();
            ASSERT_TRUE(svcRef);
            configAdmin = context.GetService(svcRef);
            ASSERT_TRUE(configAdmin);
        }
        template <typename ServiceInterfaceT>
        std::shared_ptr<ServiceInterfaceT>
        getFactoryService(std::string factoryPid, std::string factoryInstance, cppmicroservices::AnyMap const& props)
        {

            // Get a service reference to ConfigAdmin to create the factory component instance.

            if (configAdmin)
            {
                std::shared_ptr<cppmicroservices::service::cm::Configuration> factoryConfig;
                // Create factory configuration object
                if (factoryInstance.empty())
                {
                    factoryConfig = configAdmin->CreateFactoryConfiguration(factoryPid);
                }
                else
                {
                    factoryConfig = configAdmin->GetFactoryConfiguration(factoryPid, factoryInstance);
                }

                if (factoryConfig)
                {
                    auto componentName = factoryConfig->GetPid();
                    auto fut = factoryConfig->Update(props);
                    fut.get();
                    std::string filter = "(" + cppmicroservices::service::component::ComponentConstants::COMPONENT_NAME
                                         + "=" + componentName + ")";
                    auto services = context.GetServiceReferences<ServiceInterfaceT>(filter);
                    EXPECT_EQ(services.size(), 1u);
                    return context.GetService<ServiceInterfaceT>(services.at(0));
                }
            }
            return std::shared_ptr<ServiceInterfaceT>();
        }

      public:
        std::shared_ptr<cppmicroservices::service::cm::ConfigurationAdmin> configAdmin;
    };

    class MockServiceBImpl : public test::ServiceBInt
    {
      public:
        MockServiceBImpl() = default;
        ~MockServiceBImpl() = default;
    };

    /* tFactoryTarget.dependencyExistsBefore.
     * ServiceA and ServiceB are both factory instances.
     * ServiceA~1 is dependent on ServiceB~123.
     * ServiceB~123 is registered before the bundle containing ServiceA is
     * started.
     */

    TEST_F(tFactoryTarget, dependencyExistsBefore)
    {
        // Register the ServiceB~123 factory service with a mock implementation
        auto mockServiceB = std::make_shared<MockServiceBImpl>();
        cppmicroservices::ServiceProperties serviceBProps {
            {std::string("component.name"), std::string("ServiceB~123")},
            {    std::string("ServiceBId"), std::string("ServiceB~123")}
        };
        auto serviceBReg = context.RegisterService<test::ServiceBInt>(mockServiceB, serviceBProps);

        // Install and start the bundle containing the ServiceA factory.
        // DS is now listening for factory configuration objects of the form
        // ServiceA~<instance>
        test::InstallLib(context, "TestBundleDSFAC1");
        cppmicroservices::Bundle testBundle = StartTestBundle("TestBundleDSFAC1");

        // Create the ServiceA~1 configuration object. Specify a dynamic target
        // so that the test::ServiceBInt dependency for  ServiceA~1 will only be
        // satisfied by ServiceB~123 instance.
        cppmicroservices::AnyMap props;
        props["test::ServiceBInt"] = std::string("(ServiceBId=ServiceB~123)");
        auto service = getFactoryService<test::ServiceAInt>("ServiceA", "1", props);
        ASSERT_TRUE(service);

        // Clean up
        serviceBReg.Unregister();
    }

}; // namespace test
