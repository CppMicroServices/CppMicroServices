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
#ifndef _TEST_SAMPLE_HPP_
#define _TEST_SAMPLE_HPP_

#include "TestInterfaces/TestInterfacesExport.h"
#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/BundleContext.h"
#include <memory>
#include <string>

/* This file contains interface declarations for the test bundles
   used in Declarative Services Tests */

#define STRINGIZE(s)  STR_HELPER(s)
#define STR_HELPER(s) #s

namespace test
{
    class US_TestInterfaces_EXPORT Interface1
    {
      public:
        virtual std::string Description() = 0;
        virtual ~Interface1();
    };

    class US_TestInterfaces_EXPORT Interface2
    {
      public:
        virtual std::string ExtendedDescription() = 0;
        virtual ~Interface2();
    };

    class US_TestInterfaces_EXPORT Interface3
    {
      public:
        virtual bool isDependencyInjected() = 0;
        virtual ~Interface3();
    };

      class US_TestInterfaces_EXPORT Interface4
    {
      public:
        virtual bool isBound() = 0;
        virtual ~Interface4();
    };

    class US_TestInterfaces_EXPORT TestManagedServiceInterface
    {
      public:
        virtual ~TestManagedServiceInterface() = default;

        virtual int getCounter() = 0;
    };

    class US_TestInterfaces_EXPORT TestManagedServiceFactoryServiceInterface
    {
      public:
        virtual ~TestManagedServiceFactoryServiceInterface() = default;

        virtual int getValue() = 0;
    };

    class US_TestInterfaces_EXPORT TestManagedServiceFactory
    {
      public:
        TestManagedServiceFactory() = default;
        virtual ~TestManagedServiceFactory() = default;

        virtual int getUpdatedCounter(std::string const& pid) = 0;

        virtual int getRemovedCounter(std::string const& pid) = 0;

        virtual std::shared_ptr<TestManagedServiceFactoryServiceInterface> create(std::string const& config) = 0;
    };

    class US_TestInterfaces_EXPORT TestBundleDSDependent
    {
      public:
        virtual ~TestBundleDSDependent();
    };

    class US_TestInterfaces_EXPORT TestBundleDSUpstreamDependency
    {
      public:
        virtual ~TestBundleDSUpstreamDependency();
    };

    
    class US_TestInterfaces_EXPORT TestBundleDSUpstreamDependencyIsActivated
    {
      public:
        virtual ~TestBundleDSUpstreamDependencyIsActivated();
        virtual bool isActivated() = 0;
        virtual size_t numberCreated() = 0;
    };

    // Use this bundle in test bundles & test points to validate if bundle
    // static data is initialized correctly
    class US_TestInterfaces_EXPORT TestInitialization
    {
      public:
        virtual std::vector<cppmicroservices::BundleContext> GetContexts(void) = 0;
        virtual ~TestInitialization() = default;
    };

    // Interfaces for declarative services dependency graph resolution benchmarks. The
    // implentations of these interfaces have dependencies that form a complete 3 level tree:
    // DSGraph01 : DSGraph02, DSGraph03
    // DSGraph02 : DSGraph04, DSGraph05
    // DSGraph03 : DSGraph06, DSGraph07
    class US_TestInterfaces_EXPORT DSGraph01
    {
      public:
        virtual std::string Description() = 0;
        virtual ~DSGraph01();
    };

    class US_TestInterfaces_EXPORT DSGraph02
    {
      public:
        virtual std::string Description() = 0;
        virtual ~DSGraph02();
    };

    class US_TestInterfaces_EXPORT DSGraph03
    {
      public:
        virtual std::string Description() = 0;
        virtual ~DSGraph03();
    };

    class US_TestInterfaces_EXPORT DSGraph04
    {
      public:
        virtual std::string Description() = 0;
        virtual ~DSGraph04();
    };

    class US_TestInterfaces_EXPORT DSGraph05
    {
      public:
        virtual std::string Description() = 0;
        virtual ~DSGraph05();
    };

    class US_TestInterfaces_EXPORT DSGraph06
    {
      public:
        virtual std::string Description() = 0;
        virtual ~DSGraph06();
    };

    class US_TestInterfaces_EXPORT DSGraph07
    {
      public:
        virtual std::string Description() = 0;
        virtual ~DSGraph07();
    };

    // Use this interface in test bundles & test points to validate if the
    // service component receives the life cycle callbacks from the DS runtime
    class US_TestInterfaces_EXPORT LifeCycleValidation
    {
      public:
        virtual bool IsActivated() = 0;
        virtual bool IsDeactivated() = 0;
        virtual ~LifeCycleValidation();
    };

    // Use this interface in test bundles testing ConfigAdmin
    // integration into DS to get information from the component instance.
    class US_TestInterfaces_EXPORT CAInterface
    {
      public:
        virtual cppmicroservices::AnyMap GetProperties() = 0;
        virtual ~CAInterface();
    };

    // Use this interface in test bundles testing ConfigAdmin
    // integration into DS to get information from the component instance
    // and also information about dependency injection.
    class US_TestInterfaces_EXPORT CAInterface1
    {
      public:
        virtual cppmicroservices::AnyMap GetProperties() = 0;
        virtual bool isDependencyInjected() = 0;
        virtual ~CAInterface1();
    };
    // Use these interfaces in test bundles testing Factory target
    // functionality in DS.
    class US_TestInterfaces_EXPORT ServiceAInt
    {
      public:
        virtual cppmicroservices::AnyMap GetProperties() = 0;
        virtual ~ServiceAInt();

        virtual void*
        GetRefAddr() const
        {
            return nullptr;
        }
    };
    class US_TestInterfaces_EXPORT ServiceBInt
    {
      public:
        virtual cppmicroservices::AnyMap GetProperties() = 0;
        virtual ~ServiceBInt();
    };
    class US_TestInterfaces_EXPORT ServiceCInt
    {
      public:
        virtual cppmicroservices::AnyMap GetProperties() = 0;
        virtual ~ServiceCInt();
    };
} // namespace test

#endif
