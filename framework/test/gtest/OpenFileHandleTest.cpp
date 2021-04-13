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

#include <chrono>

#include "TestUtilBundleListener.h"
#include "TestUtils.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/BundleResource.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/util/FileSystem.h"
#include "gtest/gtest.h"

#if defined(US_PLATFORM_WINDOWS)
#  include "windows.h"
#elif defined(US_PLATFORM_POSIX)
#  include <stdio.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif

using namespace cppmicroservices;

static unsigned long GetHandleCountForCurrentProcess()
{
#  if defined(US_PLATFORM_WINDOWS)
  auto processHandle = GetCurrentProcess();
  unsigned long handleCount{ 0 };
  if (!GetProcessHandleCount(processHandle, &handleCount)) {
    throw std::runtime_error(
      "GetProcessHandleCount failed to retrieve the number of open handles.");
  }
  return handleCount;
#  elif defined(US_PLATFORM_POSIX)
  auto pid_t = getpid();
  std::string command("lsof -p " + std::to_string(pid_t) + " | wc -l");
  FILE* fd = popen(command.c_str(), "r");
  if (nullptr == fd) {
    throw std::runtime_error("popen failed.");
  }
  std::string result;
  char buf[PATH_MAX];
  while (nullptr != fgets(buf, PATH_MAX, fd)) {
    result += buf;
  }
  if (-1 == pclose(fd)) {
    throw std::runtime_error("pclose failed.");
  }
  return stoul(result);
#  else
  throw std::runtime_error(
    "unsupported platform - can't get handle count for current process.")
#  endif
}

// Test that the file handle count doesn't change after installing a bundle.
// Installing a bundle should not hold the file open.
TEST(OpenFileHandleTest, InstallBundle)
{
  auto f = FrameworkFactory().NewFramework();
  ASSERT_TRUE(f);
  f.Start();

  auto handleCountBefore = GetHandleCountForCurrentProcess();

#  if defined(US_BUILD_SHARED_LIBS)
  auto bundle =
    cppmicroservices::testing::InstallLib(f.GetBundleContext(), "TestBundleA");
#  else
  auto bundle =
    cppmicroservices::testing::GetBundle("TestBundleA", f.GetBundleContext());
#  endif

  auto handleCountAfter = GetHandleCountForCurrentProcess();
  ASSERT_EQ(handleCountBefore, handleCountAfter)
    << "The handle counts before and after installing a bundle should not "
       "differ.";

  f.Stop();
  f.WaitForStop(std::chrono::seconds::zero());
}

/*
  Test that the bundle properly opens and closes its resource container as
  bundle resources are requested and discarded.

  Note: This test is defined out on mac since there appears to be no reliable way
  to get the current number of open file handles on mac. Even though the system calls 
  for opening a file handle exist, it still doesn't show up in lsof and thus 
  the test always fails.
*/
TEST(OpenFileHandleTest, BundleOpenCloseContainer)
{
#if defined(US_PLATFORM_WINDOWS) || defined(US_PLATFORM_LINUX)
  auto f = FrameworkFactory().NewFramework();
  ASSERT_TRUE(f);
  f.Start();

#  if defined(US_BUILD_SHARED_LIBS)
  auto bundle =
    cppmicroservices::testing::InstallLib(f.GetBundleContext(), "TestBundleR");
#  else
  auto bundle =
    cppmicroservices::testing::GetBundle("TestBundleR", f.GetBundleContext());
#  endif

  auto handleCountAfterInstall = GetHandleCountForCurrentProcess();

  {
    // Grab a resource
    auto res = bundle.GetResource("icons/cppmicroservices.png");
    ASSERT_TRUE(res);

    // Check that the handle count is not equal to the original count
    auto handleCountAfterGetResource = GetHandleCountForCurrentProcess();
    ASSERT_NE(handleCountAfterInstall, handleCountAfterGetResource);

    {
      // Grab more resources
      auto res2 = bundle.FindResources("", "*.txt", true);
      auto handleCountAfterMultipleGetResources =
        GetHandleCountForCurrentProcess();

      // Check that the handle count is not the same as right when the
      // bundle was installed
      ASSERT_NE(handleCountAfterInstall, handleCountAfterMultipleGetResources);
    }

    // Check that the handle count is not the same as the original count
    // even after releasing some resources
    auto handleCountAfterFirstRelease = GetHandleCountForCurrentProcess();
    ASSERT_NE(handleCountAfterInstall, handleCountAfterFirstRelease);
  }

  // Check that the handle count is the same as the original after releasing
  // all references to BundleResources for the Bundle
  auto handleCountAfterFullResourceRelease = GetHandleCountForCurrentProcess();
  ASSERT_EQ(handleCountAfterInstall, handleCountAfterFullResourceRelease);

  // Shutdown framework
  f.Stop();
  f.WaitForStop(std::chrono::seconds::zero());
#endif
}
