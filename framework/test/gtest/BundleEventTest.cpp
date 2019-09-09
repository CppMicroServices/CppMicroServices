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

#include "TestUtils.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "gtest/gtest.h"

using namespace cppmicroservices;

class BundleEventTest : public ::testing::Test
{
protected:
  Bundle            bundle;
  BundleEvent       bEvt;
  FrameworkEvent    fEvt;
  BundleEvent::Type eventType;

  virtual void SetUp()
  {
    auto f = FrameworkFactory().NewFramework();
    f.Start();

#if defined(US_BUILD_SHARED_LIBS)
    bundle = 
      cppmicroservices::testing::InstallLib(f.GetBundleContext(), "TestBundleA");
#else
    bundle = 
      cppmicroservices::testing::GetBundle("TestBundleA", f.GetBundleContext());
#endif

    f.Stop();
    fEvt = f.WaitForStop(std::chrono::milliseconds::zero());
    eventType = bEvt.GetType();
  }

  virtual void TearDown()
  {}
};

TEST_F(BundleEventTest, invalidBundleEvents)
{
  const auto eventBundle = bEvt.GetBundle();
  ASSERT_THROW(BundleEvent(eventType, eventBundle),
               std::invalid_argument);
}

TEST_F(BundleEventTest, invalidBundleOrigin)
{
  const auto eventBundle = fEvt.GetBundle();
  const auto eventOrigin = bEvt.GetOrigin();
  ASSERT_THROW(BundleEvent(eventType, eventBundle, eventOrigin),
               std::invalid_argument);
}

TEST_F(BundleEventTest, bundleOrigin)
{
  const auto  eventBundle = fEvt.GetBundle();
  ASSERT_NO_THROW(BundleEvent(eventType, eventBundle, bundle));
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
