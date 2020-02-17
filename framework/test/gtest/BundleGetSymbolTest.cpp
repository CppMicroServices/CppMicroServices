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

#include "TestUtils.h"
#include "gtest/gtest.h"

#include <stdexcept>

// Enable tests only for shared libraries
#if defined(US_BUILD_SHARED_LIBS)

using namespace cppmicroservices;

class BundleGetSymbolTest : public ::testing::Test
{
protected:    
    Framework f;
    Bundle bd;

public:
    BundleGetSymbolTest() : f(FrameworkFactory().NewFramework()){}
    ~BundleGetSymbolTest() override = default;

    void SetUp() override
    {
        f.Start();
        auto bctx = f.GetBundleContext();
        bd = cppmicroservices::testing::InstallLib(bctx, "TestBundleA");
        bd.Start();
    }

    void TearDown() override
    {
        bd.Stop();
        f.Stop();
    }
};

// Test for invalid bundle
TEST_F(BundleGetSymbolTest, TestGetSymbolInvalidBundleInput)
{
    Bundle b;
    EXPECT_THROW(b.GetSymbol(nullptr,""),std::invalid_argument);
}

// Test for invalid symbol handle on valid bundle
TEST_F(BundleGetSymbolTest, TestGetSymbolValidBundleWithInvalidSymbolName)
{
    EXPECT_THROW(bd.GetSymbol(nullptr,"_us_create_activator_TestBundleA"),std::invalid_argument);
}

// Test for invalid symbol name on valid bundle
TEST_F(BundleGetSymbolTest, TestGetSymbolValidBundleWithInvalidSymbolHandle)
{
    SharedLibrary sh(bd.GetLocation());
    sh.Load();
 
    EXPECT_THROW(bd.GetSymbol(sh.GetHandle(),""),std::invalid_argument);
}

// Test for bundle which is not started but with valid inputs
TEST_F(BundleGetSymbolTest, TestGetSymbolNotInstalledBundle)
{
    auto bctx =  f.GetBundleContext();
    auto bdx = cppmicroservices::testing::InstallLib(bctx, "TestBundleA2");
    
    SharedLibrary sh(bdx.GetLocation());
    sh.Load();

    EXPECT_THROW(bdx.GetSymbol(sh.GetHandle(),"_us_create_activator_TestBundleA2"), std::runtime_error);

    sh.Unload();
}

// Test for valid bundle and valid input
TEST_F(BundleGetSymbolTest, TestGetSymbolValidInput)
{
    SharedLibrary sh(bd.GetLocation());
    sh.Load();

    EXPECT_TRUE(bd.GetSymbol(sh.GetHandle(),"_us_create_activator_TestBundleA") != nullptr) << "Error : Empty symbol returned from Bundle::GetSymbol !\n";

    sh.Unload();
}

#endif
