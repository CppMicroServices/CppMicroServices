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

#ifndef TestFixture_h
#define TestFixture_h

#include "gtest/gtest.h"

#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>

#include "../TestUtils.hpp"
#include "cppmicroservices/cm/ConfigurationAdmin.hpp"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp"
#include <chrono>

namespace test
{

    auto const DEFAULT_POLL_PERIOD = std::chrono::milliseconds(1000);

    namespace scr = cppmicroservices::service::component::runtime;

    /**
     * Test Fixture used for several test points. The setup method installs and starts
     * all the bundles found in the compendium plugins folder and the installs all the
     * test bundles.
     * This class also provides convenience methods to start a specific test bundle
     */
    class tServiceComponent : public testing::Test
    {
      public:
        tServiceComponent() : ::testing::Test(), framework(cppmicroservices::FrameworkFactory().NewFramework()) {}

        void
        TestBody() override
        {
        }

        void
        SetUp() override
        {
            framework.Start();
            context = framework.GetBundleContext();

#if defined(US_BUILD_SHARED_LIBS)
            auto dsPluginPath = test::GetDSRuntimePluginFilePath();
            auto dsbundles = context.InstallBundles(dsPluginPath);
            for (auto& bundle : dsbundles)
            {
                bundle.Start();
            }

            auto caPluginPath = test::GetConfigAdminRuntimePluginFilePath();
            auto cabundles = context.InstallBundles(caPluginPath);
            for (auto& bundle : cabundles)
            {
                bundle.Start();
            }

            test::InstallLib(context, "TestBundleDSTOI1");
            test::InstallLib(context, "TestBundleDSTOI2");
            test::InstallLib(context, "TestBundleDSTOI3");
            test::InstallLib(context, "TestBundleDSTOI5");
            test::InstallLib(context, "TestBundleDSTOI6");
            test::InstallLib(context, "TestBundleDSTOI7");
            test::InstallLib(context, "TestBundleDSTOI9");
            test::InstallLib(context, "TestBundleDSTOI10");
            test::InstallLib(context, "TestBundleDSTOI12");
            test::InstallLib(context, "TestBundleDSTOI14");
            test::InstallLib(context, "TestBundleDSTOI15");
            test::InstallLib(context, "TestBundleDSTOI16");
            test::InstallLib(context, "TestBundleDSTOI18");
            test::InstallLib(context, "TestBundleDSTOI19");
            test::InstallLib(context, "TestBundleDSTOI20");
            test::InstallLib(context, "TestBundleDSTOI21");
            test::InstallLib(context, "TestBundleDSTOI22");
            test::InstallLib(context, "TestBundleDSCA01");
            test::InstallLib(context, "TestBundleDSCA02");
            test::InstallLib(context, "TestBundleDSCA03");
            test::InstallLib(context, "TestBundleDSCA04");
            test::InstallLib(context, "TestBundleDSCA05");
            test::InstallLib(context, "TestBundleDSCA05a");
            test::InstallLib(context, "TestBundleDSCA07");
            test::InstallLib(context, "TestBundleDSCA08");
            test::InstallLib(context, "TestBundleDSCA09");
            test::InstallLib(context, "TestBundleDSCA10");
            test::InstallLib(context, "TestBundleDSCA12");
            test::InstallLib(context, "TestBundleDSCA16");
            test::InstallLib(context, "TestBundleDSCA20");
            test::InstallLib(context, "TestBundleDSCA21");
            test::InstallLib(context, "TestBundleDSCA24");
            test::InstallLib(context, "TestBundleDSCA26");
            test::InstallLib(context, "TestBundleDSCA27");
            test::InstallLib(context, "TestBundleDSCA28");
            test::InstallLib(context, "TestBundleDSFAC1");
#endif

#ifndef US_BUILD_SHARED_LIBS
            auto dsbundles = context.GetBundles();
            for (auto& bundle : dsbundles)
            {
                try
                {
                    bundle.Start();
                }
                catch (std::exception& e)
                {
                    std::cerr << "    " << e.what();
                }
                std::cerr << std::endl;
            }
#endif
            auto sRef = context.GetServiceReference<scr::ServiceComponentRuntime>();
            ASSERT_TRUE(sRef);
            dsRuntimeService = context.GetService<scr::ServiceComponentRuntime>(sRef);
            ASSERT_TRUE(dsRuntimeService);
        }

        void
        TearDown() override
        {
            framework.Stop();
            framework.WaitForStop(std::chrono::milliseconds::zero());
        }

        cppmicroservices::Bundle
        GetTestBundle(std::string const& symbolicName)
        {
            auto bundles = context.GetBundles();

            for (auto& bundle : bundles)
            {
                auto bundleSymbolicName = bundle.GetSymbolicName();
                if (symbolicName == bundleSymbolicName)
                {
                    return bundle;
                }
            }
            return cppmicroservices::Bundle();
        }

        cppmicroservices::Bundle
        StartTestBundle(std::string const& symName)
        {
            cppmicroservices::Bundle testBundle = GetTestBundle(symName);
            EXPECT_EQ(static_cast<bool>(testBundle), true);
            testBundle.Start();
            EXPECT_EQ(testBundle.GetState(), cppmicroservices::Bundle::STATE_ACTIVE)
                << " failed to start bundle with symbolic name" + symName;
            return testBundle;
        }

        template <class T>
        std::shared_ptr<T>
        GetInstance()
        {
            cppmicroservices::ServiceReference<T> instanceRef;
            std::shared_ptr<T> instance;
            instanceRef = context.GetServiceReference<T>();
            if (!instanceRef)
            {
                return std::shared_ptr<T>();
            }
            return context.GetService<T>(instanceRef);
        }

        template <class T>
        std::vector<std::shared_ptr<T>>
        GetInstances()
        {
            std::vector<cppmicroservices::ServiceReference<T>> instanceRefs;
            std::vector<std::shared_ptr<T>> instances;
            instanceRefs = context.GetServiceReferences<T>();
            if (instanceRefs.empty())
            {
                return std::vector<std::shared_ptr<T>>();
            }
            for (auto const& ref : instanceRefs)
            {
                instances.push_back(context.GetService<T>(ref));
            }
            return instances;
        }

        std::vector<scr::dto::ComponentConfigurationDTO>
        GetComponentConfigs(cppmicroservices::Bundle const& testBundle,
                            std::string const& componentName,
                            scr::dto::ComponentDescriptionDTO& compDescDTO)
        {
            compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, componentName);
            EXPECT_EQ(compDescDTO.implementationClass, componentName)
                << "Implementation class in the returned component description must be " << componentName;

            return dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
        }

        std::shared_ptr<scr::ServiceComponentRuntime> dsRuntimeService;
        cppmicroservices::Framework framework;
        cppmicroservices::BundleContext context;
    };

    /**
     * This test fixture is a generic test fixture which only installs and starts
     * DeclarativeServices. It provides helper functions, similar to tServiceComponent,
     * which help the user get and start test bundles.
     */
    class tGenericDSSuite : public testing::Test
    {
      public:
        tGenericDSSuite() : ::testing::Test(), framework(cppmicroservices::FrameworkFactory().NewFramework()) {}

        void
        TestBody() override
        {
        }

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
        }

        void
        TearDown() override
        {
            if (dsRuntimeService) {
                dsRuntimeService.reset();
            }
            framework.Stop();
            framework.WaitForStop(std::chrono::milliseconds::zero());
        }

        cppmicroservices::Bundle
        GetTestBundle(std::string const& symbolicName)
        {
            auto bundles = context.GetBundles();

            for (auto& bundle : bundles)
            {
                auto bundleSymbolicName = bundle.GetSymbolicName();
                if (symbolicName == bundleSymbolicName)
                {
                    return bundle;
                }
            }
            return cppmicroservices::Bundle();
        }

        cppmicroservices::Bundle
        StartTestBundle(std::string const& symName)
        {
            cppmicroservices::Bundle testBundle = GetTestBundle(symName);
            EXPECT_EQ(static_cast<bool>(testBundle), true);
            testBundle.Start();
            EXPECT_EQ(testBundle.GetState(), cppmicroservices::Bundle::STATE_ACTIVE)
                << " failed to start bundle with symbolic name" + symName;
            return testBundle;
        }

        template <class T>
        std::shared_ptr<T>
        GetInstance()
        {
            cppmicroservices::ServiceReference<T> instanceRef;
            std::shared_ptr<T> instance;
            instanceRef = context.GetServiceReference<T>();
            if (!instanceRef)
            {
                return std::shared_ptr<T>();
            }
            return context.GetService<T>(instanceRef);
        }

        std::vector<scr::dto::ComponentConfigurationDTO>
        GetComponentConfigs(cppmicroservices::Bundle const& testBundle,
                            std::string const& componentName,
                            scr::dto::ComponentDescriptionDTO& compDescDTO)
        {
            compDescDTO = dsRuntimeService->GetComponentDescriptionDTO(testBundle, componentName);
            EXPECT_EQ(compDescDTO.implementationClass, componentName)
                << "Implementation class in the returned component description must be " << componentName;

            return dsRuntimeService->GetComponentConfigurationDTOs(compDescDTO);
        }

        std::shared_ptr<scr::ServiceComponentRuntime> dsRuntimeService;
        cppmicroservices::Framework framework;
        cppmicroservices::BundleContext context;
    };
    class tGenericDSAndCASuite : public tGenericDSSuite
    {
      public:
        void SetUp() override
        {
            tGenericDSSuite::SetUp();
            ::test::InstallAndStartConfigAdmin(context);
            auto svcRef = context.GetServiceReference<cppmicroservices::service::cm::ConfigurationAdmin>();
            ASSERT_TRUE(svcRef);
            configAdmin = context.GetService(svcRef);
            ASSERT_TRUE(configAdmin);
        }
        void
        TearDown() override
        {
            if (configAdmin) {
                configAdmin.reset();
            }
            tGenericDSSuite::TearDown();
        }
      public:
        std::shared_ptr<cppmicroservices::service::cm::ConfigurationAdmin> configAdmin;
    };
} // namespace test

#endif /* TestFixture_h */
