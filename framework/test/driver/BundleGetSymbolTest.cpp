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
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/GetBundleContext.h"

#include "cppmicroservices/SharedLibrary.h"

#include "TestingConfig.h"
#include "TestingMacros.h"
#include "TestUtils.h"

#include <stdexcept>

using namespace cppmicroservices;

void TestGetSymbolInvalidBundleInput()
{
    // Test for invalid bundle
    
    Bundle b;
    US_TEST_FOR_EXCEPTION(std::invalid_argument , b.GetSymbol(nullptr,""));
}

void TestGetSymbolInvalidInput()
{
    FrameworkFactory factory;
    auto f = factory.NewFramework();
    f.Start();
    auto frameworkCtx = f.GetBundleContext();
    
    auto b = testing::InstallLib(frameworkCtx, "TestBundleA");
    
    SharedLibrary sh(b.GetLocation());
    sh.Load();
    
    // Test for invalid inputs on valid bundle
 
    US_TEST_FOR_EXCEPTION(std::invalid_argument , b.GetSymbol(sh.GetHandle(),""));
    US_TEST_FOR_EXCEPTION(std::invalid_argument , b.GetSymbol(nullptr,"_us_create_activator_TestBundleA"));
    
    b.Stop();
    f.Stop();
}


void TestGetSymbolNotInstalledBundle()
{
    FrameworkFactory factory;
    auto f = factory.NewFramework();
    f.Start();
    auto frameworkCtx = f.GetBundleContext();
    auto b = testing::InstallLib(frameworkCtx, "TestBundleA");

    SharedLibrary sh(b.GetLocation());
    sh.Load();
   
    // Test for bundle which is not installed
    US_TEST_FOR_EXCEPTION(std::runtime_error , b.GetSymbol(sh.GetHandle(),"_us_create_activator_TestBundleA"));

    b.Stop();
    sh.Unload();
    f.Stop();
}

void TestGetSymbolValidInput()
{
    FrameworkFactory factory;
    auto f = factory.NewFramework();
    f.Start();
    auto frameworkCtx = f.GetBundleContext();
    auto b = testing::InstallLib(frameworkCtx, "TestBundleA");
    b.Start();

    SharedLibrary sh(b.GetLocation());
    sh.Load();

    // Test for bundle which is not installed inputs
    US_TEST_CONDITION(b.GetSymbol(sh.GetHandle(),"_us_create_activator_TestBundleA") != nullptr,"Error : Empty symbol returned from Bundle::GetSymbol !");

    b.Stop();
    sh.Unload();
    f.Stop();
}

int BundleGetSymbolTest(int /*argc*/, char* /*argv*/ [])
{
    US_TEST_BEGIN("BundleGetSymbolTest");

   
    TestGetSymbolInvalidInput();
    TestGetSymbolNotInstalledBundle();
    TestGetSymbolValidInput();

    US_TEST_END()
}
