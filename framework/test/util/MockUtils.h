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

#ifndef CPPMICROSERVICES_MOCKUTILS_H
#define CPPMICROSERVICES_MOCKUTILS_H

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/ListenerToken.h"
#include "cppmicroservices/ServiceEvent.h"

#include "FrameworkPrivate.h"
#include "CoreBundleContext.h"
#include "BundleStorage.h"
#include "BundleContextPrivate.h"
#include "../gtest/Mocks.h"

namespace cppmicroservices
{

    struct CoreBundleContextHolder;

    /*
     * A helper class for writing tests with mocks. Instantiates a concrete
     * Framework, but mocks any filesystem accesses and makes underlying
     * private classes accessible.
     */
    class MockedEnvironment
    {
      public:
        MockedEnvironment(bool expectFrameworkStart = true);
        std::vector<Bundle> Install(std::string& location,
                                    cppmicroservices::AnyMap& bundleManifest,
                                    std::shared_ptr<MockBundleResourceContainer> const& resCont);

        Framework framework;
        CoreBundleContext* coreBundleContext;
        BundlePrivate* bundlePrivate;
        BundleContext bundleContext;
        BundleContextPrivate* bundleContextPrivate;

        MockBundleStorageMemory* bundleStorage;
        MockBundleRegistry* bundleRegistry;
    };

    struct MockTestBundleAService
    {
        virtual ~MockTestBundleAService() {}
    };

    struct MockTestBundleA : public cppmicroservices::MockTestBundleAService
    {
        virtual ~MockTestBundleA() {}
    };

    class MockTestBundleAActivator : public cppmicroservices::BundleActivator
    {
      public:
        MockTestBundleAActivator() {}
        ~MockTestBundleAActivator() {}

        void Start(BundleContext context)
        {
            s = std::make_shared<MockTestBundleA>();
            sr = context.RegisterService<MockTestBundleAService>(s);
        }

        void Stop(BundleContext)
        {
            sr.Unregister();
        }

      private:
        std::shared_ptr<MockTestBundleA> s;
        ServiceRegistration<MockTestBundleAService> sr;
    };

    struct MockTestBundleBActivator : public cppmicroservices::BundleActivator
    {
      public:
        void Start(BundleContext)
        {}

        void Stop(BundleContext)
        {}
    };

    struct MockTestBundleImportedByBActivator : public cppmicroservices::BundleActivator
    {
      public:
        void Start(BundleContext)
        {}

        void Stop(BundleContext)
        {}
    };

    struct MockTestBundleUActivator : public cppmicroservices::BundleActivator
    {
      public:
        void Start(BundleContext)
        {}

        void Stop(BundleContext)
        {}
    };

    template<typename T>
    BundleActivator* createActivator();
    void destroyActivator(BundleActivator* bundleActivator);

    static std::map<std::string, BundleActivator*(*)()> activators = {
        { "TestBundleA", &createActivator<MockTestBundleAActivator> },
        { "TestBundleA2", &createActivator<MockTestBundleAActivator> },
        { "TestBundleB", &createActivator<MockTestBundleBActivator> },
        { "TestBundleImportedByB", &createActivator<MockTestBundleImportedByBActivator> },
        { "TestBundleU", &createActivator<MockTestBundleUActivator> }
    };

} // namespace cppmicroservices

#endif // CPPMICROSERVICES_MOCKUTILS_H
