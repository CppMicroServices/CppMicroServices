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

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/ServiceFactory.h"

#include "gtest/gtest.h"

struct Interface1
{
};
struct Interface2
{
};
struct Interface3
{
};

struct MyService1 : public Interface1
{
};

struct MyService2
    : public Interface1
    , public Interface2
{
};

struct MyService3
    : public Interface1
    , public Interface2
    , public Interface3
{
};

struct MyFactory1 : public cppmicroservices::ServiceFactory
{
    std::map<long, std::shared_ptr<MyService1>> m_idToServiceMap;

    virtual cppmicroservices::InterfaceMapConstPtr
    GetService(cppmicroservices::Bundle const& bundle,
               cppmicroservices::ServiceRegistrationBase const& /*registration*/)
    {
        auto s = std::make_shared<MyService1>();
        m_idToServiceMap.insert(std::make_pair(bundle.GetBundleId(), s));
        return cppmicroservices::MakeInterfaceMap<Interface1>(s);
    }

    virtual void
    UngetService(cppmicroservices::Bundle const& bundle,
                 cppmicroservices::ServiceRegistrationBase const& /*registration*/,
                 cppmicroservices::InterfaceMapConstPtr const& service)
    {
        auto iter = m_idToServiceMap.find(bundle.GetBundleId());
        if (iter != m_idToServiceMap.end())
        {
            // Compare service pointer
            EXPECT_EQ((iter->second), cppmicroservices::ExtractInterface<Interface1>(service));
            m_idToServiceMap.erase(iter);
        }
    }
};

struct MyFactory2 : public cppmicroservices::ServiceFactory
{
    std::map<long, std::shared_ptr<MyService2>> m_idToServiceMap;

    virtual cppmicroservices::InterfaceMapConstPtr
    GetService(cppmicroservices::Bundle const& bundle,
               cppmicroservices::ServiceRegistrationBase const& /*registration*/)
    {
        auto s = std::make_shared<MyService2>();
        m_idToServiceMap.insert(std::make_pair(bundle.GetBundleId(), s));
        return cppmicroservices::MakeInterfaceMap<Interface1, Interface2>(s);
    }

    virtual void
    UngetService(cppmicroservices::Bundle const& bundle,
                 cppmicroservices::ServiceRegistrationBase const& /*registration*/,
                 cppmicroservices::InterfaceMapConstPtr const& service)
    {
        auto iter = m_idToServiceMap.find(bundle.GetBundleId());
        if (iter != m_idToServiceMap.end())
        {
            // Compare service pointer
            EXPECT_EQ((iter->second), cppmicroservices::ExtractInterface<Interface2>(service));
            m_idToServiceMap.erase(iter);
        }
    }
};

struct MyFactory3 : public cppmicroservices::ServiceFactory
{
    std::map<long, std::shared_ptr<MyService3>> m_idToServiceMap;

    virtual cppmicroservices::InterfaceMapConstPtr
    GetService(cppmicroservices::Bundle const& bundle,
               cppmicroservices::ServiceRegistrationBase const& /*registration*/)
    {
        auto s = std::make_shared<MyService3>();
        m_idToServiceMap.insert(std::make_pair(bundle.GetBundleId(), s));
        return cppmicroservices::MakeInterfaceMap<Interface1, Interface2, Interface3>(s);
    }

    virtual void
    UngetService(cppmicroservices::Bundle const& bundle,
                 cppmicroservices::ServiceRegistrationBase const& /*registration*/,
                 cppmicroservices::InterfaceMapConstPtr const& service)
    {
        auto iter = m_idToServiceMap.find(bundle.GetBundleId());
        if (iter != m_idToServiceMap.end())
        {
            // Compare service pointer
            EXPECT_EQ((iter->second), cppmicroservices::ExtractInterface<Interface3>(service));
            m_idToServiceMap.erase(iter);
        }
    }
};

using namespace cppmicroservices;

TEST(ServiceTemplateTest, testServiceTemplate)
{
    FrameworkFactory factory;
    auto framework = factory.NewFramework();
    framework.Start();

    auto context = framework.GetBundleContext();

    // Register compile tests
    auto s1 = std::make_shared<MyService1>();
    auto s2 = std::make_shared<MyService2>();
    auto s3 = std::make_shared<MyService3>();

    auto sr1 = context.RegisterService<Interface1>(s1);
    auto sr2 = context.RegisterService<Interface1, Interface2>(s2);
    auto sr3 = context.RegisterService<Interface1, Interface2, Interface3>(s3);

    auto f1 = std::make_shared<MyFactory1>();
    auto sfr1 = context.RegisterService<Interface1>(ToFactory(f1));

    auto f2 = std::make_shared<MyFactory2>();
    auto sfr2 = context.RegisterService<Interface1, Interface2>(ToFactory(f2));

    auto f3 = std::make_shared<MyFactory3>();
    auto sfr3 = context.RegisterService<Interface1, Interface2, Interface3>(ToFactory(f3));

#ifdef US_BUILD_SHARED_LIBS
    ASSERT_EQ(context.GetBundle().GetRegisteredServices().size(), 6);
#endif

    auto s1refs = context.GetServiceReferences<Interface1>();
    // check # of interface1 regs
    ASSERT_EQ(s1refs.size(), 6);
    auto s2refs = context.GetServiceReferences<Interface2>();
    // Check # of interface2 regs
    ASSERT_EQ(s2refs.size(), 4);
    auto s3refs = context.GetServiceReferences<Interface3>();
    // Check # of interface3 regs
    ASSERT_EQ(s3refs.size(), 2);

    auto i1 = context.GetService(sr1.GetReference());
    ASSERT_EQ(i1, s1);
    i1.reset();

    i1 = context.GetService(sfr1.GetReference());
    // test interface1 factory ptr
    ASSERT_EQ(i1, f1->m_idToServiceMap[context.GetBundle().GetBundleId()]);
    i1.reset();

    i1 = context.GetService(sr2.GetReference<Interface1>());
    ASSERT_EQ(i1, s2);
    i1.reset();

    i1 = context.GetService(sfr2.GetReference<Interface1>());
    ASSERT_EQ(i1, f2->m_idToServiceMap[context.GetBundle().GetBundleId()]);
    i1.reset();

    auto i2 = context.GetService(sr2.GetReference<Interface2>());
    ASSERT_EQ(i2, s2);
    i2.reset();

    i2 = context.GetService(sfr2.GetReference<Interface2>());
    ASSERT_EQ(i2, f2->m_idToServiceMap[context.GetBundle().GetBundleId()]);
    i2.reset();

    i1 = context.GetService(sr3.GetReference<Interface1>());
    ASSERT_EQ(i1, s3);
    i1.reset();

    i1 = context.GetService(sfr3.GetReference<Interface1>());
    ASSERT_EQ(i1, f3->m_idToServiceMap[context.GetBundle().GetBundleId()]);
    i1.reset();

    i2 = context.GetService(sr3.GetReference<Interface2>());
    ASSERT_EQ(i2, s3);
    i2.reset();

    i2 = context.GetService(sfr3.GetReference<Interface2>());
    ASSERT_EQ(i2, f3->m_idToServiceMap[context.GetBundle().GetBundleId()]);
    i2.reset();

    auto i3 = context.GetService(sr3.GetReference<Interface3>());
    ASSERT_EQ(i3, s3);
    i3.reset();

    i3 = context.GetService(sfr3.GetReference<Interface3>());
    ASSERT_EQ(i3, f3->m_idToServiceMap[context.GetBundle().GetBundleId()]);
    i3.reset();

    sr1.Unregister();
    sr2.Unregister();
    sr3.Unregister();
}
