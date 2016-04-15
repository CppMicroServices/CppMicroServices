/*=============================================================================
 
 Library: CppMicroServices
 
 Copyright (c) The CppMicroServices developers. See the COPYRIGHT
 file at the top-level directory of this distribution and at
 https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .
 
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

#include "usTestingMacros.h"
#include <usFrameworkFactory.h>
#include <usFramework.h>
#include <usBundle.h>
#include <usBundleContext.h>
#include "usTestingConfig.h"

#include <string>

using namespace us;

/**
 * Test point to ensure the executable bundle is installed properly.
 */
int usExecutableBundleTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ExecutableBundleTest");
  
  auto framework = FrameworkFactory().NewFramework();
  framework->Start();
  BundleContext* frameworkCtx = framework->GetBundleContext();
  auto bundle = frameworkCtx->InstallBundle(BIN_PATH + DIR_SEP + "usCoreTestDriver" + EXE_EXT + "/main");
  US_TEST_CONDITION_REQUIRED(bundle != nullptr, "Bundle found in current executable");
  US_TEST_CONDITION_REQUIRED(bundle->GetName() == "main", "Check bundle name");
  US_TEST_CONDITION_REQUIRED(bundle->GetProperty(Bundle::PROP_VERSION).ToString() == "0.1.0", "Check bundle version");
  
  US_TEST_END()
}