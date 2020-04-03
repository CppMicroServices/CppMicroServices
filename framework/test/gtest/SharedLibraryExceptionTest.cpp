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

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/SharedLibraryException.h"

#include "TestUtils.h"

#include "gtest/gtest.h"

#if defined (US_BUILD_SHARED_LIBS)
TEST(SharedLibraryException, FrameworkSharedLibraryException)
{
  auto f = cppmicroservices::FrameworkFactory().NewFramework();
  ASSERT_TRUE(f);
  f.Start();
  
  ASSERT_NO_THROW((void)cppmicroservices::testing::InstallLib(f.GetBundleContext(), "TestBundleSLE1"));
  auto bundle = cppmicroservices::testing::InstallLib(f.GetBundleContext(), "TestBundleSLE1");
  ASSERT_THROW(bundle.Start(), cppmicroservices::SharedLibraryException);
  
  f.Stop();
  f.WaitForStop(std::chrono::milliseconds::zero());
}
#endif //US_BUILD_SHARED_LIBS
