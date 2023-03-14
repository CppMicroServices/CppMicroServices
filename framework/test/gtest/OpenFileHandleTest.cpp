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
#include "TestingConfig.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/util/FileSystem.h"
#include "gtest/gtest.h"

#include "miniz.h"

#if defined(US_PLATFORM_WINDOWS)
#    include "windows.h"
#elif defined(US_PLATFORM_POSIX)
#    include <stdio.h>
#    include <sys/types.h>
#    include <unistd.h>
#endif

using namespace cppmicroservices;

static unsigned long
GetHandleCountForCurrentProcess()
{
#if defined(US_PLATFORM_WINDOWS)
    auto processHandle = GetCurrentProcess();
    unsigned long handleCount { 0 };
    if (!GetProcessHandleCount(processHandle, &handleCount))
    {
        throw std::runtime_error("GetProcessHandleCount failed to retrieve the number of open handles.");
    }
    return handleCount;
#elif defined(US_PLATFORM_POSIX)
    auto pid_t = getpid();
    std::string command("lsof -p " + std::to_string(pid_t) + " | wc -l");
    FILE* fd = popen(command.c_str(), "r");
    if (nullptr == fd)
    {
        throw std::runtime_error("popen failed.");
    }
    std::string result;
    char buf[PATH_MAX];
    while (nullptr != fgets(buf, PATH_MAX, fd))
    {
        result += buf;
    }
    if (-1 == pclose(fd))
    {
        throw std::runtime_error("pclose failed.");
    }
    return stoul(result);
#else
    throw std::runtime_error("unsupported platform - can't get handle count for current process.")
#endif
}

// Test that the file handle count doesn't change after installing a bundle.
// Installing a bundle should not hold the file open.
TEST(OpenFileHandleTest, InstallBundle)
{
    auto f = FrameworkFactory().NewFramework();
    ASSERT_TRUE(f);
    f.Start();

    auto handleCountBefore = GetHandleCountForCurrentProcess();

#if defined(US_BUILD_SHARED_LIBS)
    auto bundle = cppmicroservices::testing::InstallLib(f.GetBundleContext(), "TestBundleA");
#else
    auto bundle = cppmicroservices::testing::GetBundle("TestBundleA", f.GetBundleContext());
#endif

    auto handleCountAfter = GetHandleCountForCurrentProcess();
    ASSERT_EQ(handleCountBefore, handleCountAfter)
        << "The handle counts before and after installing a bundle should not "
           "differ.";

    f.Stop();
    f.WaitForStop(std::chrono::seconds::zero());
}

// Test that file handles do not leak when a bundle fails to install.
TEST(OpenFileHandleTest, InstallBundleFailure)
{
    auto f = FrameworkFactory().NewFramework();
    ASSERT_TRUE(f);
    ASSERT_NO_THROW(f.Start());
    auto context = f.GetBundleContext();

    auto baselineHandleCount = GetHandleCountForCurrentProcess();

    // Test that bogus bundle installs throw the appropriate exception and don't
    // leak file handles.
    EXPECT_THROW(context.InstallBundles(std::string {}), std::runtime_error);

    auto newHandleCounter = GetHandleCountForCurrentProcess();
    EXPECT_EQ(baselineHandleCount, newHandleCounter);

    EXPECT_THROW(context.InstallBundles(std::string("\\path\\which\\won't\\exist\\phantom_bundle")),
                 std::runtime_error);

    newHandleCounter = GetHandleCountForCurrentProcess();
    EXPECT_EQ(baselineHandleCount, newHandleCounter);

#if defined(US_BUILD_SHARED_LIBS)

    // Using miniz APIs to qualify that the test module has the correct
    // embedded zip file.
    mz_zip_archive zipArchive;
    memset(&zipArchive, 0, sizeof(mz_zip_archive));

    std::string testModulePath = cppmicroservices::testing::LIB_PATH + util::DIR_SEP + US_LIB_PREFIX
                                 + "TestModuleWithEmbeddedZip" + US_LIB_POSTFIX + US_LIB_EXT;
    EXPECT_TRUE(mz_zip_reader_init_file(&zipArchive, testModulePath.c_str(), 0));

    mz_uint numFiles = mz_zip_reader_get_num_files(const_cast<mz_zip_archive*>(&zipArchive));
    EXPECT_EQ(numFiles, 1) << "Wrong # of files in the zip found.";
    for (mz_uint fileIndex = 0; fileIndex < numFiles; ++fileIndex)
    {
        char fileName[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
        if (mz_zip_reader_get_filename(&zipArchive, fileIndex, fileName, MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE))
        {
            std::string strFileName { fileName };
            std::size_t pos = strFileName.find_first_of('/');
            EXPECT_EQ(pos, std::string::npos) << "Found a directory in the zip file where one should not exist.";
        }
    }

    mz_zip_reader_end(&zipArchive);

    // Test that a shared library which contains zip formatted data not in
    // the format CppMicroServices expects fails correctly and does not leak
    // file handles.
    EXPECT_THROW(cppmicroservices::testing::InstallLib(f.GetBundleContext(), "TestModuleWithEmbeddedZip");
                 , std::runtime_error);

    newHandleCounter = GetHandleCountForCurrentProcess();
    EXPECT_EQ(baselineHandleCount, newHandleCounter);
#endif

    f.Stop();
    f.WaitForStop(std::chrono::seconds::zero());
}
