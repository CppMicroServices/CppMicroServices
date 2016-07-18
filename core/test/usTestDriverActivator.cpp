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

#include "usTestDriverActivator.h"
#include "usBundleImport.h"

namespace us {

TestDriverActivator* TestDriverActivator::m_Instance = nullptr;

TestDriverActivator::TestDriverActivator()
  : m_StartCalled(false)
{
}

bool TestDriverActivator::StartCalled()
{
  return m_Instance ? m_Instance->m_StartCalled : false;
}

void TestDriverActivator::Start(BundleContext)
{
  this->m_Instance = this;
  this->m_StartCalled = true;
}

void TestDriverActivator::Stop(BundleContext)
{
  this->m_Instance = nullptr;
}

}

US_EXPORT_BUNDLE_ACTIVATOR(us::TestDriverActivator)

#ifndef US_BUILD_SHARED_LIBS
US_INITIALIZE_STATIC_BUNDLE(system_bundle)
US_IMPORT_BUNDLE(TestBundleA)
US_IMPORT_BUNDLE(TestBundleA2)
US_IMPORT_BUNDLE(TestBundleB)
US_IMPORT_BUNDLE(TestBundleLQ)
#ifdef US_ENABLE_THREADING_SUPPORT
US_IMPORT_BUNDLE(TestBundleC1)
#endif
US_IMPORT_BUNDLE(TestBundleH)
US_IMPORT_BUNDLE(TestBundleM)
US_INITIALIZE_STATIC_BUNDLE(TestBundleR)
US_INITIALIZE_STATIC_BUNDLE(TestBundleRA)
US_INITIALIZE_STATIC_BUNDLE(TestBundleRL)
US_IMPORT_BUNDLE(TestBundleS)
US_IMPORT_BUNDLE(TestBundleSL1)
US_IMPORT_BUNDLE(TestBundleSL3)
US_IMPORT_BUNDLE(TestBundleSL4)
US_IMPORT_BUNDLE(main)
#endif
