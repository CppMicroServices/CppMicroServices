#include "TestFixture.hpp"
#include "gtest/gtest.h"

#include "../TestUtils.hpp"
#include "TestInterfaces/Interfaces.hpp"

namespace test
{
    struct IntTestService
    {
        virtual ~IntTestService() {}
    };

    struct implTestService : IntTestService
    {
    };

    /**
     * Verify a component with an interface with :: leading can be registered and found
     */
    TEST(TestGlobalNamespacing, testServiceWithGlobalNamespaceNoDS)
    {
        auto framework = cppmicroservices::FrameworkFactory().NewFramework();
        framework.Start();
        auto context = framework.GetBundleContext();

        auto s1 = std::make_shared<::test::implTestService>();
        auto reg1 = context.RegisterService<::test::implTestService>(s1);
        auto ref1 = context.GetServiceReference<test::implTestService>();
        ASSERT_TRUE(ref1);

        auto ref2 = context.GetServiceReference<::test::implTestService>();
        ASSERT_TRUE(ref2);
    }

    TEST(TestGlobalNamespacing, testServiceWithGlobalNamespaceDS)
    {
        auto framework = cppmicroservices::FrameworkFactory().NewFramework();
        framework.Start();
        auto context = framework.GetBundleContext();

        test::InstallAndStartDS(context);

        auto bundle = ::test::InstallAndStartBundle(context, "GloballyNamespacedInterfaces");

        ASSERT_TRUE(bundle);
        auto ref0 = context.GetServiceReference<test::GlobalNS1>();
        auto ref1 = context.GetServiceReference<test::GlobalNS1>();
        ASSERT_TRUE(ref0);
        ASSERT_TRUE(ref1);
        auto svc0 = context.GetService(ref0);
        auto svc1 = context.GetService(ref1);
        ASSERT_EQ(svc0->Description(), "globalService1");
        ASSERT_EQ(svc1->Description(), "globalService1");
        ASSERT_TRUE(context.GetServiceReference<test::GlobalNS2>());
        ASSERT_TRUE(context.GetServiceReference<::test::GlobalNS2>());
    }

    // verify that a DS test which is namespaced globally can be retrieved with either :: or not
    // verify that a DS test that REFERENCES an interfaced that is globally namespaced
} // namespace test