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

#include "usCoreConfig.h"

namespace us {


std::map<std::string, Any> InitProperties(std::map<std::string, Any> configuration)
{
    // emplace cannot be used until the minimum GCC compiler is >= 4.8
    configuration.insert(std::make_pair(Framework::PROP_STORAGE_LOCATION, Any(GetCurrentWorkingDirectory())));
    configuration.insert(std::pair<std::string, Any>(Framework::PROP_LOG_LEVEL, 3));

    // Framework::PROP_THREADING_SUPPORT is a read-only property whose value is based off of a compile-time switch.
    // Run-time modification of the property should be ignored as it is irrelevant.
    configuration.erase(Framework::PROP_THREADING_SUPPORT);
#ifdef US_ENABLE_THREADING_SUPPORT
    configuration.insert(std::pair<std::string, Any>(Framework::PROP_THREADING_SUPPORT, std::string("multi")));
#else
    configuration.insert(std::pair<std::string, Any>(Framework::PROP_THREADING_SUPPORT, std::string("single")));
#endif

    return std::move(configuration);
}

FrameworkPrivate::FrameworkPrivate(Framework* qq, const BundleInfo& info, const std::map<std::string, Any>& configuration)
  : BundlePrivate(qq, info)
  , coreBundleContext(qq, InitProperties(configuration))
{
  Logger::instance().SetLogLevel(static_cast<us::MsgType>(any_cast<int>(coreBundleContext.frameworkProperties.find(Framework::PROP_LOG_LEVEL)->second)));
}

}
