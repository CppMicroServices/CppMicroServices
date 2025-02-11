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

#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "TestUtils.h"
#include "gtest/gtest.h"

#include <unordered_set>

using namespace cppmicroservices;

struct ITestServiceA
{
    virtual ~ITestServiceA() {}
};

struct ITestServiceB
{
    virtual ~ITestServiceB() {};
};

struct TestServiceA : public ITestServiceA
{
};

// Test the optional macro to provide custom name for a service interface class
CPPMICROSERVICES_DECLARE_SERVICE_INTERFACE(ITestServiceB, "com.mycompany.ITestService/1.0");

class ServiceRegistryTest : public ::testing::Test
{
  protected:
    Framework framework;
    BundleContext context;

  public:
    ServiceRegistryTest() : framework(FrameworkFactory().NewFramework()) {};

    ~ServiceRegistryTest() override = default;

    void
    SetUp() override
    {
        framework.Start();
        context = framework.GetBundleContext();
    }

    void
    TearDown() override
    {
        framework.Stop();
        framework.WaitForStop(std::chrono::milliseconds::zero());
    }
};

TEST_F(ServiceRegistryTest, TestServiceInterfaceId)
{
    ASSERT_EQ(us_service_interface_iid<int>(), "int");
    ASSERT_EQ(us_service_interface_iid<ITestServiceA>(), "ITestServiceA");
    ASSERT_EQ(us_service_interface_iid<::ITestServiceA>(), "ITestServiceA");
    ASSERT_EQ(us_service_interface_iid<ITestServiceB>(), "com.mycompany.ITestService/1.0");
}

TEST_F(ServiceRegistryTest, TestGlobalNamespaceOnServiceRegistration)
{
    auto s1 = std::make_shared<TestServiceA>();

    ServiceRegistration<ITestServiceA> reg1 = context.RegisterService<::ITestServiceA>(s1);

    // Test for two registered ITestServiceA services
    ServiceReference<ITestServiceA> ref = context.GetServiceReference("ITestServiceA");
    ASSERT_TRUE(ref);

    ref = context.GetServiceReference("::ITestServiceA");
    ASSERT_TRUE(ref);

    // Test for no ITestServiceA services
    reg1.Unregister();
    ref = context.GetServiceReference("::ITestServiceA");
    ASSERT_FALSE(ref);

    // Test for invalid service reference
    ref = context.GetServiceReference<ITestServiceA>();
    ASSERT_FALSE(ref);
}

TEST_F(ServiceRegistryTest, TestMultipleServiceRegistrations)
{
    auto s1 = std::make_shared<TestServiceA>();
    auto s2 = std::make_shared<TestServiceA>();

    ServiceRegistration<ITestServiceA> reg1 = context.RegisterService<ITestServiceA>(s1);
    ServiceRegistration<ITestServiceA> reg2 = context.RegisterService<ITestServiceA>(s2);

    // Test for two registered ITestServiceA services
    std::vector<ServiceReference<ITestServiceA>> refs = context.GetServiceReferences<ITestServiceA>();
    ASSERT_EQ(refs.size(), 2);

    // Test for one registered ITestServiceA services
    reg2.Unregister();
    refs = context.GetServiceReferences<ITestServiceA>();
    ASSERT_EQ(refs.size(), 1);

    // Test for no ITestServiceA services
    reg1.Unregister();
    refs = context.GetServiceReferences<ITestServiceA>();
    ASSERT_TRUE(refs.empty());

    // Test for invalid service reference
    ServiceReference<ITestServiceA> ref = context.GetServiceReference<ITestServiceA>();
    ASSERT_FALSE(ref);
}

TEST_F(ServiceRegistryTest, TestUnregisterFix)
{
    ServiceRegistration<int> registration = context.RegisterService<int>(std::make_shared<int>());
    ServiceReference<int> reference = context.GetServiceReference<int>();
    registration.Unregister();
    ASSERT_FALSE(reference.IsConvertibleTo("IBooService"));
}

TEST_F(ServiceRegistryTest, TestServiceReferenceMemberFunctions)
{
    auto s1 = std::make_shared<TestServiceA>();
    ServiceProperties props;
    props["StringKey"] = std::string("A string value");
    props["Status"] = false;

    ServiceRegistration<ITestServiceA> reg1 = context.RegisterService<ITestServiceA>(s1, props);
    ServiceReference<ITestServiceA> ref1 = context.GetServiceReference<ITestServiceA>();

    // Test ServiceReference member functions GetPropertyKeys()
    std::vector<std::string> keys;
    ref1.GetPropertyKeys(keys);
    ASSERT_NE(std::find(keys.begin(), keys.end(), "StringKey"), keys.end());
    ASSERT_NE(std::find(keys.begin(), keys.end(), "Status"), keys.end());

    auto keys_by_val = ref1.GetPropertyKeys();
    ASSERT_EQ(keys_by_val, keys);

    // Test the ostream<< operator of ServiceReference
    std::ostringstream strstream;
    strstream << ref1;
    ASSERT_GT(static_cast<int>(strstream.str().size()), 0);

    // Test the ostream<< operator of an invalid ServiceReference
    ServiceReference<ITestServiceA> invalid_ref;
    std::ostringstream strstream2;
    strstream2 << invalid_ref;
    ASSERT_EQ(strstream2.str(), "Invalid service reference");

    // Test the custom hash function of ServiceReference
    std::unordered_set<ServiceReferenceBase> sr_ref_set;
    sr_ref_set.insert(ref1);
    sr_ref_set.insert(invalid_ref);
    ASSERT_EQ(sr_ref_set.size(), 2);

    reg1.Unregister();
}

TEST_F(ServiceRegistryTest, TestServicePropertiesUpdate)
{
    auto s1 = std::make_shared<TestServiceA>();
    ServiceProperties props;
    props["string"] = std::string("A std::string");
    props["bool"] = false;
    char const* str = "A const char*";
    props["const char*"] = str;

    ServiceRegistration<ITestServiceA> reg1 = context.RegisterService<ITestServiceA>(s1, props);
    ServiceReference<ITestServiceA> ref1 = context.GetServiceReference<ITestServiceA>();

    ASSERT_EQ(context.GetServiceReferences<ITestServiceA>().size(), 1);
    ASSERT_FALSE(any_cast<bool>(ref1.GetProperty("bool")));

    // register second service with higher rank
    auto s2 = std::make_shared<TestServiceA>();
    ServiceProperties props2;
    props2[Constants::SERVICE_RANKING] = 50;

    ServiceRegistration<ITestServiceA> reg2 = context.RegisterService<ITestServiceA>(s2, props2);

    // Get the service with the highest rank, this should be s2.
    ServiceReference<ITestServiceA> ref2 = context.GetServiceReference<ITestServiceA>();
    auto service = std::dynamic_pointer_cast<TestServiceA>(context.GetService(ref2));
    ASSERT_EQ(service, s2);

    props["bool"] = true;
    // change the service ranking
    props[Constants::SERVICE_RANKING] = 100;
    reg1.SetProperties(props);

    ASSERT_EQ(context.GetServiceReferences<ITestServiceA>().size(), 2);
    ASSERT_TRUE(any_cast<bool>(ref1.GetProperty("bool")));
    ASSERT_EQ(any_cast<int>(ref1.GetProperty(Constants::SERVICE_RANKING)), 100);

    // Service with the highest ranking should now be s1
    service = std::dynamic_pointer_cast<TestServiceA>(context.GetService<ITestServiceA>(ref1));
    ASSERT_EQ(service, s1);

    reg1.Unregister();
    ASSERT_EQ(context.GetServiceReferences<ITestServiceA>("").size(), 1);

    service = std::dynamic_pointer_cast<TestServiceA>(context.GetService<ITestServiceA>(ref2));
    ASSERT_EQ(service, s2);

    reg2.Unregister();
    ASSERT_TRUE(context.GetServiceReferences<ITestServiceA>().empty());
}
