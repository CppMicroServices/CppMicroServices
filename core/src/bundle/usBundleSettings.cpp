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

#include "usBundleSettings.h"
#include "usThreads_p.h"
#include "usWaitCondition_p.h"

#include <string>
#include <sstream>
#include <set>
#include <algorithm>
#include <cctype>

namespace us {

namespace {

  std::string RemoveTrailingPathSeparator(const std::string& in)
  {
#ifdef US_PLATFORM_WINDOWS
    const char separator = '\\';
#else
    const char separator = '/';
#endif
    if (in.empty()) return in;
    std::string::const_iterator lastChar = --in.end();
    while (lastChar != in.begin() && std::isspace(*lastChar)) lastChar--;
    if (*lastChar != separator) lastChar++;
    std::string::const_iterator firstChar = in.begin();
    while (firstChar < lastChar && std::isspace(*firstChar)) firstChar++;
    return std::string(firstChar, lastChar);
  }

}

std::string BundleSettings::CURRENT_BUNDLE_PATH()
{
  static const std::string var = "us_current_bundle_path";
  return var;
}

struct BundleSettingsPrivate : public MultiThreaded<>
{
  BundleSettingsPrivate()
    : autoLoadPaths()
  #ifdef US_ENABLE_AUTOLOADING_SUPPORT
    , autoLoadingEnabled(true)
  #else
    , autoLoadingEnabled(false)
  #endif
    , autoLoadingDisabled(false)
  {
    autoLoadPaths.insert(BundleSettings::CURRENT_BUNDLE_PATH());

    char* envPaths = getenv("US_AUTOLOAD_PATHS");
    if (envPaths != nullptr)
    {
      std::stringstream ss(envPaths);
      std::string envPath;
#ifdef US_PLATFORM_WINDOWS
      const char separator = ';';
#else
      const char separator = ':';
#endif
      while (std::getline(ss, envPath, separator))
      {
        std::string normalizedEnvPath = RemoveTrailingPathSeparator(envPath);
        if (!normalizedEnvPath.empty())
        {
          extraPaths.insert(normalizedEnvPath);
        }
      }
    }

    if (getenv("US_DISABLE_AUTOLOADING"))
    {
      autoLoadingDisabled = true;
    }
  }

  std::set<std::string> autoLoadPaths;
  std::set<std::string> extraPaths;
  bool autoLoadingEnabled;
  bool autoLoadingDisabled;
};

BundleSettings::BundleSettings() :
    pimpl(new BundleSettingsPrivate())
{

}

BundleSettings::~BundleSettings()
{
  if(pimpl)
  {
    delete pimpl;
  }
}

bool BundleSettings::IsAutoLoadingEnabled()
{
#ifdef US_ENABLE_AUTOLOADING_SUPPORT
  return (pimpl->Lock(), !pimpl->autoLoadingDisabled && pimpl->autoLoadingEnabled);
#else
  return false;
#endif
}

void BundleSettings::SetAutoLoadingEnabled(bool enable)
{
  pimpl->Lock(), pimpl->autoLoadingEnabled = enable;
}

BundleSettings::PathList BundleSettings::GetAutoLoadPaths()
{
  auto l = pimpl->Lock();
  BundleSettings::PathList paths(pimpl->autoLoadPaths.begin(),
                                 pimpl->autoLoadPaths.end());
  paths.insert(paths.end(), pimpl->extraPaths.begin(),
               pimpl->extraPaths.end());
  std::sort(paths.begin(), paths.end());
  paths.erase(std::unique(paths.begin(), paths.end()), paths.end());
  return paths;
}

void BundleSettings::SetAutoLoadPaths(const PathList& paths)
{
  PathList normalizedPaths;
  normalizedPaths.resize(paths.size());
  std::transform(paths.begin(), paths.end(), normalizedPaths.begin(), RemoveTrailingPathSeparator);

  auto l = pimpl->Lock();
  pimpl->autoLoadPaths.clear();
  pimpl->autoLoadPaths.insert(normalizedPaths.begin(), normalizedPaths.end());
}

void BundleSettings::AddAutoLoadPath(const std::string& path)
{
  pimpl->Lock(), pimpl->autoLoadPaths.insert(RemoveTrailingPathSeparator(path));
}

}
