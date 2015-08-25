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

#include "usFrameworkPrivate_p.h"

#include "usCoreBundleContext_p.h"
#include "usThreads_p.h"

US_BEGIN_NAMESPACE

FrameworkPrivate::FrameworkPrivate(void) :
    initLock(new Mutex()),
    coreBundleContext(new CoreBundleContext()),
    initialized(false)
{

}

FrameworkPrivate::FrameworkPrivate(std::map<std::string, std::string>& configuration) :
    initLock(new Mutex()),
    coreBundleContext(new CoreBundleContext()),
    initialized(false)
{
  coreBundleContext->frameworkProperties = configuration;
}

FrameworkPrivate::~FrameworkPrivate()
{
  if (coreBundleContext)
  {
    delete coreBundleContext;
  }

  if (initLock)
  {
    delete initLock;
  }

  initialized = false;
}

US_END_NAMESPACE
