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

#include <string>
#include <memory>

/* This file contains interface declarations for the test bundles
   used in Declarative Services Tests */

#define STRINGIZE(s) STR_HELPER(s)
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
}

#endif
