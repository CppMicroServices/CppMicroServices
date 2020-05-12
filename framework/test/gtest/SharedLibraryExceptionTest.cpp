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

#include "cppmicroservices/SharedLibraryException.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/SharedLibrary.h"

#include "TestUtils.h"

#include "gtest/gtest.h"

#if defined(US_BUILD_SHARED_LIBS)

class SharedLibraryExceptionTest : public ::testing::Test
{
protected:
  SharedLibraryExceptionTest()
    : f(cppmicroservices::FrameworkFactory().NewFramework())
  {}

  virtual ~SharedLibraryExceptionTest() = default;

  virtual void SetUp()
  {
    f.Start();
    ASSERT_NO_THROW((void)cppmicroservices::testing::InstallLib(
      f.GetBundleContext(), "TestBundleSLE1"));
    b = cppmicroservices::testing::InstallLib(f.GetBundleContext(),
                                              "TestBundleSLE1");
  }

  virtual void TearDown()
  {
    f.Stop();
    f.WaitForStop(std::chrono::milliseconds::zero());
  }

  cppmicroservices::Bundle& GetBundle() { return b; }

private:
  cppmicroservices::Framework f;
  cppmicroservices::Bundle b;
};

TEST_F(SharedLibraryExceptionTest, testSharedLibraryFailure)
{
  // Call SharedLibrary::Load() on framework to get a std::system_error
  // exception thrown.
  auto lib = cppmicroservices::SharedLibrary("invalidpath");
  ASSERT_THROW(lib.Load(), std::system_error);
}

TEST_F(SharedLibraryExceptionTest, testConstructSharedLibraryException)
{
  auto bundle = GetBundle();
  std::error_code errco(0, std::system_category());
  std::string exception_str("Test String");

  auto ex =
    cppmicroservices::SharedLibraryException(errco, exception_str, bundle);

  ASSERT_EQ(ex.code(), errco);
  ASSERT_EQ(std::string(ex.what()).rfind(exception_str, 0),
            0); // make sure the exception message starts with exception_str
  ASSERT_EQ(ex.GetBundle(), bundle);
}

TEST_F(SharedLibraryExceptionTest, testFrameworkSharedLibraryExceptionHandling)
{
  auto bundle = GetBundle();

  try {
    bundle.Start(); // should throw cppmicroservices::SharedLibraryException
    FAIL() << "Exception should have been caught from bundle.Start()";
  } catch (const cppmicroservices::SharedLibraryException& ex) {
    // origin bundle captured by SharedLibraryException should
    // point to the bundle that threw during Start()
    ASSERT_TRUE(bundle == ex.GetBundle());
  } catch (...) {
    FAIL()
      << "SharedLibraryException expected, but a different exception caught.";
  }
}
#endif //US_BUILD_SHARED_LIBS
