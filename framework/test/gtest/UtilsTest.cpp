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

#include "../../src/util/Utils.h"
#include "../../src/bundle/BundleUtils.h"

#include <TestUtils.h>

#include <gtest/gtest.h>


using namespace cppmicroservices;
using namespace cppmicroservices::testing;

class UtilsFs : public ::testing::Test
{
public:

  static void SetUpTestCase()
  {
    TempDir = cppmicroservices::testing::MakeUniqueTempDirectory();
  }

  static void TearDownTestCase()
  {
    // Avoid valgrind memcheck errors with gcc 4.6 by explicitly
    // cleaning up the directory, instead of relying on static
    // destruction.
    TempDir = cppmicroservices::testing::TempDir();
  }

protected:

  static std::string GetTooLongPath()
  {
#ifdef US_PLATFORM_WINDOWS
    const long name_max = MAX_PATH;
#else
    const long name_max = pathconf("/", _PC_NAME_MAX);
#endif
    if (name_max < 1)
      return std::string();

    std::vector<char> longName(name_max + 2, 'x');
    longName[name_max + 1] = '\0';
    return TempDir.Path + DIR_SEP + longName.data();
  }

  static std::string GetExistingDir()
  {
    return fs::GetCurrentWorkingDirectory();
  }

  static std::string GetExistingFile()
  {
    return BundleUtils::GetExecutablePath();
  }

  static cppmicroservices::testing::TempDir TempDir;
};

cppmicroservices::testing::TempDir UtilsFs::TempDir;

TEST_F(UtilsFs, Exists)
{
  EXPECT_FALSE(fs::Exists("should not exist"));
  EXPECT_TRUE(fs::Exists(GetExistingDir())) << "Test for existing directory";
  EXPECT_TRUE(fs::Exists(GetExistingFile())) << "Test for existing file";

  auto longPath = GetTooLongPath();
  if (!longPath.empty())
  {
#ifdef US_PLATFORM_WINDOWS
    EXPECT_FALSE(fs::Exists(longPath));
#else
    EXPECT_THROW({ fs::Exists(longPath); }, std::invalid_argument);
#endif
  }
}

TEST_F(UtilsFs, IsDirectory)
{
  EXPECT_FALSE(fs::IsDirectory("not a directory"));
  EXPECT_TRUE(fs::IsDirectory(GetExistingDir()));
  EXPECT_FALSE(fs::IsDirectory(GetExistingFile()));

  auto longPath = GetTooLongPath();
  if (!longPath.empty())
  {
#ifdef US_PLATFORM_WINDOWS
    EXPECT_FALSE(fs::IsDirectory(longPath));
#else
    EXPECT_THROW({ fs::IsDirectory(longPath); }, std::invalid_argument);
#endif
  }
}

TEST_F(UtilsFs, IsFile)
{
  EXPECT_FALSE(fs::IsFile("not a file"));
  EXPECT_FALSE(fs::IsFile(GetExistingDir()));
  EXPECT_TRUE(fs::IsFile(GetExistingFile()));

  auto longPath = GetTooLongPath();
  if (!longPath.empty())
  {
#ifdef US_PLATFORM_WINDOWS
    EXPECT_FALSE(fs::IsFile(longPath));
#else
    EXPECT_THROW({ fs::IsFile(longPath); }, std::invalid_argument);
#endif
  }
}

TEST_F(UtilsFs, IsRelative)
{
  EXPECT_TRUE(fs::IsRelative(""));
  EXPECT_TRUE(fs::IsRelative("rel"));
  EXPECT_FALSE(fs::IsRelative(GetExistingFile()));
}

TEST_F(UtilsFs, GetAbsolute)
{
  EXPECT_EQ(fs::GetAbsolute("rel", GetExistingDir()), GetExistingDir() + DIR_SEP + "rel");
  EXPECT_EQ(fs::GetAbsolute(GetExistingFile(), "dummy"), GetExistingFile());
}

TEST_F(UtilsFs, MakeAndRemovePath)
{
  // Try to make a path that contains an existing file as a sub-path
  const File filePath = MakeUniqueTempFile(TempDir);
  const std::string invalidPath = filePath.Path + DIR_SEP + "invalid";
  EXPECT_THROW(fs::MakePath(invalidPath), std::invalid_argument);
  EXPECT_THROW(fs::RemoveDirectoryRecursive(invalidPath), std::invalid_argument);

  // Test path name limits
  const std::string tooLongPath = GetTooLongPath();
  EXPECT_THROW(fs::MakePath(tooLongPath), std::invalid_argument);
  EXPECT_THROW(fs::RemoveDirectoryRecursive(tooLongPath), std::invalid_argument);

  // Create a valid path
  const std::string validPath = TempDir.Path + DIR_SEP + "one" + DIR_SEP + "two";
  ASSERT_NO_THROW(fs::MakePath(validPath));
  ASSERT_NO_THROW(fs::RemoveDirectoryRecursive(validPath));
}

