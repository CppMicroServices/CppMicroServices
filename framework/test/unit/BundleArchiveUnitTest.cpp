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

// White-box tests for the framework-private BundleArchive class, using a
// gmock BundleStorage. Neither class is exported from CppMicroServices.

#include "BundleArchive.h"
#include "mocks/MockBundleStorage.h"

#include "cppmicroservices/BundleResource.h"

#include "gtest/gtest.h"

using ::testing::Return;
using ::testing::StrictMock;

namespace cppmicroservices
{
    namespace
    {
        std::shared_ptr<BundleArchive>
        MakeArchive(BundleStorage* storage, long id)
        {
            AnyMap manifest(AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);
            manifest["bundle.symbolic_name"] = std::string("mock_bundle");
            return std::make_shared<BundleArchive>(storage,
                                                   nullptr,
                                                   "mock_bundle",
                                                   "/path/to/mock_bundle.dll",
                                                   id,
                                                   manifest);
        }
    } // namespace

    TEST(BundleArchiveUnitTest, PurgeRemovesArchiveFromOwningStorage)
    {
        StrictMock<test::MockBundleStorage> storage;
        auto archive = MakeArchive(&storage, 7);

        EXPECT_CALL(storage, RemoveArchive(archive.get())).WillOnce(Return(true));
        archive->Purge();
    }

    TEST(BundleArchiveUnitTest, AccessorsReflectConstructionArguments)
    {
        StrictMock<test::MockBundleStorage> storage;
        auto archive = MakeArchive(&storage, 42);

        EXPECT_TRUE(archive->IsValid());
        EXPECT_EQ(42, archive->GetBundleId());
        EXPECT_EQ("/path/to/mock_bundle.dll", archive->GetBundleLocation());
        EXPECT_EQ("mock_bundle", archive->GetResourcePrefix());
        EXPECT_EQ(-1, archive->GetAutostartSetting());
        EXPECT_EQ("mock_bundle", any_cast<std::string>(archive->GetInjectedManifest().at("bundle.symbolic_name")));
    }

    TEST(BundleArchiveUnitTest, NegativeIdIsInvalid)
    {
        StrictMock<test::MockBundleStorage> storage;
        EXPECT_FALSE(MakeArchive(&storage, -1)->IsValid());
    }

    TEST(BundleArchiveUnitTest, NoResourceContainerYieldsNoResources)
    {
        StrictMock<test::MockBundleStorage> storage;
        auto archive = MakeArchive(&storage, 1);

        EXPECT_FALSE(archive->GetResource("/manifest.json"));
        EXPECT_TRUE(archive->FindResources("/", "*", true).empty());
        EXPECT_EQ(nullptr, archive->GetResourceContainer());
    }

    TEST(BundleArchiveUnitTest, AutostartSettingRoundTrips)
    {
        StrictMock<test::MockBundleStorage> storage;
        auto archive = MakeArchive(&storage, 1);

        archive->SetAutostartSetting(3);
        EXPECT_EQ(3, archive->GetAutostartSetting());
    }
} // namespace cppmicroservices
