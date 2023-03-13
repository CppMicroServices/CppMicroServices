#ifndef TEST_FIXTURES_HPP_
#define TEST_FIXTURES_HPP_

#include "gtest/gtest.h"

#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>

#include "boost/asio/async_result.hpp"
#include "boost/asio/packaged_task.hpp"
#include "boost/asio/post.hpp"
#include "boost/asio/thread_pool.hpp"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp"

#include "ConfigurationAdminTestingConfig.h"

namespace scr = cppmicroservices::service::component::runtime;
namespace cm = cppmicroservices::service::cm;

namespace
{
    std::string
    PathToLib(std::string const& libName)
    {
        return (cppmicroservices::testing::LIB_PATH + cppmicroservices::util::DIR_SEP + US_LIB_PREFIX + libName
                + US_LIB_POSTFIX + US_LIB_EXT);
    }

    std::string
    GetDSRuntimePluginFilePath()
    {
        std::string libName { "DeclarativeServices" };
#if defined(US_PLATFORM_WINDOWS)
        // This is a hack for the time being.
        // TODO: revisit changing the hard-coded "1" to the DS version dynamically
        libName += "1";
#endif
        return PathToLib(libName);
    }

    std::string
    GetConfigAdminRuntimePluginFilePath()
    {
        std::string libName { "ConfigurationAdmin" };
#if defined(US_PLATFORM_WINDOWS)
        libName += US_ConfigurationAdmin_VERSION_MAJOR;
#endif
        return PathToLib(libName);
    }

    void
    InstallAndStartDSAndConfigAdmin(::cppmicroservices::BundleContext& ctx)
    {
        std::vector<cppmicroservices::Bundle> DSBundles;
        std::vector<cppmicroservices::Bundle> CMBundles;
        auto dsPluginPath = GetDSRuntimePluginFilePath();
        auto cmPluginPath = GetConfigAdminRuntimePluginFilePath();

#if defined(US_BUILD_SHARED_LIBS)
        DSBundles = ctx.InstallBundles(dsPluginPath);
        CMBundles = ctx.InstallBundles(cmPluginPath);
#else
        DSBundles = ctx.GetBundles();
        CMBundles = ctx.GetBundles();
#endif

        for (auto b : DSBundles)
        {
            b.Start();
        }

        for (auto b : CMBundles)
        {
            b.Start();
        }
    }
} // namespace

/**
 * This test fixture is a generic test fixture which only installs and starts
 * DeclarativeServices and ConfigAdmin.
 */
class tGenericDSAndCASuite : public testing::Test
{
  public:
    tGenericDSAndCASuite() : ::testing::Test(), framework(cppmicroservices::FrameworkFactory().NewFramework()) {}

    void
    TestBody() override
    {
    }

    void
    SetUp() override
    {
        framework.Start();
        context = framework.GetBundleContext();

        InstallAndStartDSAndConfigAdmin(context);

        auto sRef = context.GetServiceReference<scr::ServiceComponentRuntime>();
        ASSERT_TRUE(sRef);
        dsRuntimeService = context.GetService<scr::ServiceComponentRuntime>(sRef);
        ASSERT_TRUE(dsRuntimeService);

        auto sr = context.GetServiceReference<cppmicroservices::service::cm::ConfigurationAdmin>();
        ASSERT_TRUE(sr);
        configAdmin = context.GetService<cm::ConfigurationAdmin>(sr);
        ASSERT_TRUE(configAdmin);
    }

    void
    TearDown() override
    {
        configAdmin.reset();

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

  public:
    std::shared_ptr<scr::ServiceComponentRuntime> dsRuntimeService;
    std::shared_ptr<cppmicroservices::service::cm::ConfigurationAdmin> configAdmin;
    cppmicroservices::Framework framework;
    cppmicroservices::BundleContext context;
};

#endif
