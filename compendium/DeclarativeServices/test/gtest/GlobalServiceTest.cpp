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
    TEST_F(tServiceComponent, testServiceWithGlobalNamespaceNoDS)
    {
        auto context = framework.GetBundleContext();

        auto s1 = std::make_shared<::test::implTestService>();
        auto reg1 = context.RegisterService<::test::implTestService>(s1);
        auto ref1 = context.GetServiceReference<test::implTestService>();
        ASSERT_TRUE(ref1);

        auto ref2 = context.GetServiceReference<::test::implTestService>();
        ASSERT_TRUE(ref2);
    }

    TEST_F(tServiceComponent, testServiceWithGlobalNamespaceDS)
    {
        auto context = framework.GetBundleContext();

        test::InstallAndStartDS(context);

        auto bundle = ::test::InstallAndStartBundle(context, "GloballyNamespacedInterfaces");

        ASSERT_TRUE(bundle);
        // auto ref = context.GetServiceReference<test::GlobalNS1>();
        // ASSERT_TRUE(ref);
        // auto svc = context.GetService(ref);
        // ASSERT_EQ(svc->Description(), "globalService1");
        ASSERT_TRUE(context.GetServiceReference<test::GlobalNS2>());
        // ASSERT_TRUE(context.GetServiceReference<::test::GlobalNS1>());
        ASSERT_TRUE(context.GetServiceReference<::test::GlobalNS2>());
    }

    // verify that a DS test whcih is namespaced globally can be retrieved with either ;; or not
    // verify that a DS test that REFERENCES an interfaced that is globally namespaced
} // namespace test