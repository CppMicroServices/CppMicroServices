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

#include "usFrameworkPrivate.h"
#include "usFramework.h"
#include "usLog.h"
#include "usUtils_p.h"

namespace us {

    FrameworkPrivate::FrameworkPrivate(void) :
        coreBundleContext(),
        initialized(false)
    {
        Init();
    }

    FrameworkPrivate::FrameworkPrivate(const std::map<std::string, std::string>& configuration) :
        coreBundleContext(),
        initialized(false)
    {
        coreBundleContext.frameworkProperties = configuration;
        Init();
    }

    FrameworkPrivate::~FrameworkPrivate()
    {
        initialized = false;
    }

    void FrameworkPrivate::Init()
    {
        // emplace cannot be used until the minimum GCC compiler is >= 4.8
        coreBundleContext.frameworkProperties.insert(std::pair<std::string, std::string>(Framework::PROP_STORAGE_LOCATION, GetCurrentWorkingDirectory()));
        coreBundleContext.frameworkProperties.insert(std::pair<std::string, std::string>(Framework::PROP_LOG_LEVEL, "3"));

        // Framework::PROP_THREADING_SUPPORT is a read-only property whose value is based off of a compile-time switch.
        // Run-time modification of the property should be ignored as it is irrelevant.
        coreBundleContext.frameworkProperties.erase(Framework::PROP_THREADING_SUPPORT);
#ifdef US_ENABLE_THREADING_SUPPORT
        coreBundleContext.frameworkProperties.insert(std::pair<std::string, std::string>(Framework::PROP_THREADING_SUPPORT, "multi"));
#else
        coreBundleContext.frameworkProperties.insert(std::pair<std::string, std::string>(Framework::PROP_THREADING_SUPPORT, "single"));
#endif

        Logger::instance().SetLogLevel(static_cast<us::MsgType>(std::stoul(coreBundleContext.frameworkProperties.find(Framework::PROP_LOG_LEVEL)->second)));
    }

}
