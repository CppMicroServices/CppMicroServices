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

#include "cppmicroservices/SharedLibrary.h"
#include "TestUtils.h"
#include "TestingConfig.h"
#include "cppmicroservices/util/FileSystem.h"
#include "gtest/gtest.h"

using namespace cppmicroservices;

TEST(SharedLibraryTest, TestSharedLibraryOperations)
{
    auto lib1Name = "TestBundleA" US_LIB_POSTFIX;
    const std::string libAFilePath
        = cppmicroservices::testing::LIB_PATH + util::DIR_SEP + US_LIB_PREFIX + lib1Name + US_LIB_EXT;
    SharedLibrary lib1(libAFilePath);

    ASSERT_EQ(lib1.GetFilePath(), libAFilePath);
    ASSERT_EQ(lib1.GetLibraryPath(), cppmicroservices::testing::LIB_PATH);
    ASSERT_EQ(lib1.GetName(), lib1Name);
    ASSERT_EQ(lib1.GetPrefix(), US_LIB_PREFIX);
    ASSERT_EQ(lib1.GetSuffix(), US_LIB_EXT);

    lib1.SetName("bla");
    ASSERT_EQ(lib1.GetName(), lib1Name);
    lib1.SetLibraryPath("bla");
    ASSERT_EQ(lib1.GetLibraryPath(), cppmicroservices::testing::LIB_PATH);
    lib1.SetPrefix("bla");
    ASSERT_EQ(lib1.GetPrefix(), US_LIB_PREFIX);
    lib1.SetSuffix("bla");
    ASSERT_EQ(lib1.GetSuffix(), US_LIB_EXT);
    ASSERT_EQ(lib1.GetFilePath(), libAFilePath);

    lib1.SetFilePath("bla");
    ASSERT_EQ(lib1.GetFilePath(), "bla");
    ASSERT_TRUE(lib1.GetLibraryPath().empty());
    ASSERT_EQ(lib1.GetName(), "bla");
    ASSERT_EQ(lib1.GetPrefix(), US_LIB_PREFIX);
    ASSERT_EQ(lib1.GetSuffix(), US_LIB_EXT);

    ASSERT_THROW(lib1.Load(), std::runtime_error);
    ASSERT_EQ(lib1.IsLoaded(), false);
    ASSERT_EQ(lib1.GetHandle(), nullptr);

    lib1.SetFilePath(libAFilePath);
    lib1.Load();
    ASSERT_TRUE(lib1.IsLoaded());
    ASSERT_NE(lib1.GetHandle(), nullptr);
    ASSERT_THROW(lib1.Load(), std::logic_error);

    lib1.SetFilePath("bla");
    ASSERT_EQ(lib1.GetFilePath(), libAFilePath);
    lib1.Unload();

    auto lib2Name = "TestBundleA" US_LIB_POSTFIX;
    SharedLibrary lib2(cppmicroservices::testing::LIB_PATH, lib2Name);
    ASSERT_EQ(lib2.GetFilePath(), libAFilePath);
    lib2.SetPrefix("");
    ASSERT_TRUE(lib2.GetPrefix().empty());
    ASSERT_EQ(lib2.GetFilePath(),
              (cppmicroservices::testing::LIB_PATH + util::DIR_SEP + "TestBundleA" + US_LIB_POSTFIX + US_LIB_EXT));

    SharedLibrary lib3 = lib2;
    ASSERT_EQ(lib3.GetFilePath(), lib2.GetFilePath());
    lib3.SetPrefix(US_LIB_PREFIX);
    ASSERT_EQ(lib3.GetFilePath(), libAFilePath);
    lib3.Load();
    ASSERT_TRUE(lib3.IsLoaded());
    ASSERT_TRUE(lib2.IsLoaded());
    lib1 = lib3;
    ASSERT_THROW(lib1.Load(), std::logic_error);

    lib3.Unload();
    ASSERT_FALSE(lib3.IsLoaded());
    ASSERT_FALSE(lib1.IsLoaded());

// gcov on Mac OS X writes coverage files during static destruction
// resulting in a crash if a dylib is completely unloaded from the process.
// https://bugs.llvm.org/show_bug.cgi?id=27224
#if !defined(US_PLATFORM_APPLE) || !defined(US_COVERAGE_ENABLED)
    lib2.Unload();
    ASSERT_FALSE(lib2.IsLoaded());
#endif
    lib1.Unload();
}
