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

#ifndef CPPMICROSERVICES_TEST_MOCKBUNDLESTORAGE_H
#define CPPMICROSERVICES_TEST_MOCKBUNDLESTORAGE_H

#include "BundleStorage.h"

#include "gmock/gmock.h"

namespace cppmicroservices
{
    namespace test
    {
        // gmock implementation of the framework-private BundleStorage interface.
        // BundleStorage is not exported from the CppMicroServices library, so
        // this mock is only usable from tests that link the framework objects
        // directly (usFrameworkUnitTests).
        class MockBundleStorage : public BundleStorage
        {
          public:
            MOCK_METHOD(std::shared_ptr<BundleArchive>,
                        CreateAndInsertArchive,
                        (std::shared_ptr<BundleResourceContainer> const&, std::string const&, ManifestT const&),
                        (override));
            MOCK_METHOD(std::vector<std::shared_ptr<BundleArchive>>, GetAllBundleArchives, (), (const, override));
            MOCK_METHOD(std::vector<long>, GetStartOnLaunchBundles, (), (const, override));
            MOCK_METHOD(void, Close, (), (override));

            // RemoveArchive is private in BundleStorage (only BundleArchive may
            // call it). gmock can still override and verify it.
            MOCK_METHOD(bool, RemoveArchive, (BundleArchive const*), (override));
        };
    } // namespace test
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_TEST_MOCKBUNDLESTORAGE_H
