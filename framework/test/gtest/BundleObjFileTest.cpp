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

#include "cppmicroservices/util/BundleObjFactory.h"
#include "cppmicroservices/util/BundleObjFile.h"

#include "cppmicroservices/util/FileSystem.h"

#include "TestUtils.h"
#include "TestingConfig.h"

#include "gtest/gtest.h"

namespace {
#if defined (US_BUILD_SHARED_LIBS)
const std::string testBundlePath = cppmicroservices::testing::LIB_PATH + cppmicroservices::util::DIR_SEP + US_LIB_PREFIX + "TestBundleRL" + US_LIB_EXT;
#else
const std::string testBundlePath = cppmicroservices::testing::BIN_PATH + cppmicroservices::util::DIR_SEP + "usFrameworkTests" + US_EXE_EXT;
#endif
}

TEST(BundleObjFile, InvalidLocation)
{
  ASSERT_THROW(cppmicroservices::BundleObjFactory().CreateBundleFileObj("/does/not/exist/bogus.bundle"),
    cppmicroservices::InvalidObjFileException);
}

TEST(BundleObjFile, InvalidBinaryFileFormat)
{
  cppmicroservices::testing::File tempFile = cppmicroservices::testing::MakeUniqueTempFile(cppmicroservices::testing::GetTempDirectory());
  std::string invalidFileFormat(tempFile.Path);
  ASSERT_TRUE(cppmicroservices::util::Exists(invalidFileFormat)) << invalidFileFormat + " should exist on disk.";
  ASSERT_THROW(cppmicroservices::BundleObjFactory().CreateBundleFileObj(invalidFileFormat),
    cppmicroservices::InvalidObjFileException);
}

TEST(BundleObjFile, NonStandardBundleExt)
{
#if defined (US_BUILD_SHARED_LIBS)
  std::string nonStandardExtBundlePath(cppmicroservices::testing::LIB_PATH + cppmicroservices::util::DIR_SEP + US_LIB_PREFIX + "TestBundleExt.cppms");
  ASSERT_TRUE(cppmicroservices::util::Exists(nonStandardExtBundlePath)) << nonStandardExtBundlePath + " should exist on disk.";
  ASSERT_NO_THROW(cppmicroservices::BundleObjFactory().CreateBundleFileObj(nonStandardExtBundlePath));
#endif
}

TEST(BundleObjFile, GetRawBundleResourceContainer)
{
#if defined (US_BUILD_SHARED_LIBS)
  ASSERT_TRUE(cppmicroservices::util::Exists(testBundlePath)) << testBundlePath + " should exist on disk.";
  ASSERT_NO_THROW({
    auto bundleObj = cppmicroservices::BundleObjFactory().CreateBundleFileObj(testBundlePath);
    auto data = bundleObj->GetRawBundleResourceContainer();

    ASSERT_TRUE(data);
    ASSERT_GT(data->GetSize(), 0u);
  });
#endif
}

