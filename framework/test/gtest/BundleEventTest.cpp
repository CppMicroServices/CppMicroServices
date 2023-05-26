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

#include "cppmicroservices/BundleEvent.h"
#include "TestUtils.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "gtest/gtest.h"

using namespace cppmicroservices;

class BundleEventTest : public ::testing::Test
{
  protected:
    Bundle bundle;
    Framework f;

  public:
    BundleEventTest() : f(FrameworkFactory().NewFramework()) {};

    ~BundleEventTest() override = default;

    void
    SetUp() override
    {
        f.Start();

#if defined(US_BUILD_SHARED_LIBS)
        bundle = cppmicroservices::testing::InstallLib(f.GetBundleContext(), "TestBundleA");
#else
        bundle = cppmicroservices::testing::GetBundle("TestBundleA", f.GetBundleContext());
#endif
    }

    void
    TearDown() override
    {
        f.Stop();
        f.WaitForStop(std::chrono::milliseconds::zero());
    }
};

TEST_F(BundleEventTest, invalidBundleEvents)
{
    Bundle b;
    ASSERT_THROW(BundleEvent(BundleEvent::Type::BUNDLE_INSTALLED, b), std::invalid_argument);
}

TEST_F(BundleEventTest, invalidBundleOrigin)
{
    Bundle b;
    ASSERT_THROW(BundleEvent(BundleEvent::Type::BUNDLE_INSTALLED, bundle, b), std::invalid_argument);
}

TEST_F(BundleEventTest, validBundleOrigin)
{
    ASSERT_NO_THROW(BundleEvent(BundleEvent::Type::BUNDLE_INSTALLED, bundle, bundle));
}

TEST_F(BundleEventTest, StreamLazyActivationBundleEventType)
{
    std::stringstream buffer;
    std::streambuf* backup = std::cout.rdbuf(buffer.rdbuf());
    std::cout << BundleEvent(BundleEvent::Type::BUNDLE_LAZY_ACTIVATION, bundle);
    std::string actStr = buffer.str();
    ASSERT_TRUE(actStr.find("LAZY_ACTIVATION") != std::string::npos);
    std::cout.rdbuf(backup);
}

TEST_F(BundleEventTest, StreamUnknownBundleEventType)
{
    std::stringstream buffer;
    std::streambuf* backup = std::cout.rdbuf(buffer.rdbuf());
    std::cout << BundleEvent(BundleEvent::Type::BUNDLE_UPDATED, bundle);
    std::string actStr = buffer.str();
    ASSERT_TRUE(actStr.find("Unknown bundle event type") != std::string::npos);
    std::cout.rdbuf(backup);
}

TEST_F(BundleEventTest, BundleEventValidation)
{
    ASSERT_EQ(BundleEvent::Type::BUNDLE_INSTALLED, static_cast<BundleEvent::Type>(1));
    ASSERT_EQ(BundleEvent::Type::BUNDLE_STARTED, static_cast<BundleEvent::Type>(2));
    ASSERT_EQ(BundleEvent::Type::BUNDLE_STOPPED, static_cast<BundleEvent::Type>(4));
    ASSERT_EQ(BundleEvent::Type::BUNDLE_UPDATED, static_cast<BundleEvent::Type>(8));
    ASSERT_EQ(BundleEvent::Type::BUNDLE_UNINSTALLED, static_cast<BundleEvent::Type>(16));
    ASSERT_EQ(BundleEvent::Type::BUNDLE_RESOLVED, static_cast<BundleEvent::Type>(32));
    ASSERT_EQ(BundleEvent::Type::BUNDLE_UNRESOLVED, static_cast<BundleEvent::Type>(64));
    ASSERT_EQ(BundleEvent::Type::BUNDLE_STARTING, static_cast<BundleEvent::Type>(128));
    ASSERT_EQ(BundleEvent::Type::BUNDLE_STOPPING, static_cast<BundleEvent::Type>(256));
    ASSERT_EQ(BundleEvent::Type::BUNDLE_LAZY_ACTIVATION, static_cast<BundleEvent::Type>(512));

    BundleEvent invalid_event;
    ASSERT_TRUE(!invalid_event);
    ASSERT_EQ(BundleEvent::Type::BUNDLE_UNINSTALLED, invalid_event.GetType());
    ASSERT_TRUE(!invalid_event.GetBundle());
    ASSERT_TRUE(!invalid_event.GetOrigin());

    BundleEvent valid_event(BundleEvent::Type::BUNDLE_UNINSTALLED, f);
    ASSERT_TRUE(valid_event);
    ASSERT_EQ(BundleEvent::Type::BUNDLE_UNINSTALLED, valid_event.GetType());
    ASSERT_EQ(f, valid_event.GetBundle());

    BundleEvent dup_valid_event(valid_event);
    ASSERT_EQ(valid_event, dup_valid_event);

    dup_valid_event = invalid_event;
    ASSERT_EQ(invalid_event, dup_valid_event);

    dup_valid_event = BundleEvent(BundleEvent::Type::BUNDLE_INSTALLED, f);
    ASSERT_TRUE(!(dup_valid_event == invalid_event));
}