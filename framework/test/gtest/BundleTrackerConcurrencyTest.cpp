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
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleTracker.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "TestUtils.h"
#include "gtest/gtest.h"

#ifdef GetObject
#  undef GetObject
#endif

using namespace cppmicroservices;

class BundleTrackerConcurrencyTest : public ::testing::Test
{
protected:
  Framework framework;
  BundleContext context;
  BundleTracker<>::BundleState all_states =
    _CreateStateMask(Bundle::State::STATE_ACTIVE,
                     Bundle::State::STATE_INSTALLED,
                     Bundle::State::STATE_RESOLVED,
                     Bundle::State::STATE_STARTING,
                     Bundle::State::STATE_STOPPING,
                     Bundle::State::STATE_UNINSTALLED);

public:
  BundleTrackerConcurrencyTest()
    : framework(FrameworkFactory().NewFramework()){};

  ~BundleTrackerConcurrencyTest() override = default;

  void SetUp() override
  {
    framework.Start();
    context = framework.GetBundleContext();
  }

  void TearDown() override
  {
    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }
};

TEST_F(BundleTrackerConcurrencyTest, ConcurrentOpenCloseWorks)
{
  //TODO
}

TEST_F(BundleTrackerConcurrencyTest, NoRaceConditionForRemovingChangingBundle)
{
  //TODO
}