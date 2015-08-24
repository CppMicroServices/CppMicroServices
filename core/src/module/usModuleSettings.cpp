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

#include "usModuleSettings.h"
#include "usThreads_p.h"
#include "usWaitCondition_p.h"

#include <string>
#include <sstream>
#include <set>
#include <algorithm>
#include <cctype>

US_BEGIN_NAMESPACE

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

std::string ModuleSettings::CURRENT_MODULE_PATH()
{
  static const std::string var = "us_current_module_path";
  return var;
}

struct ModuleSettingsPrivate : public MultiThreaded<>
{
  ModuleSettingsPrivate()
    : autoLoadPaths()
  #ifdef US_ENABLE_AUTOLOADING_SUPPORT
    , autoLoadingEnabled(true)
  #else
    , autoLoadingEnabled(false)
  #endif
    , autoLoadingDisabled(false)
  {
    autoLoadPaths.insert(ModuleSettings::CURRENT_MODULE_PATH());

    char* envPaths = getenv("US_AUTOLOAD_PATHS");
    if (envPaths != NULL)
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

ModuleSettings::ModuleSettings() :
    pimpl(new ModuleSettingsPrivate())
{

}
 
ModuleSettings::~ModuleSettings()
{
  if(pimpl)
  {
    delete pimpl;
  }
}

bool ModuleSettings::IsThreadingSupportEnabled()
{
#ifdef US_ENABLE_THREADING_SUPPORT
  return true;
#else
  return false;
#endif
}

bool ModuleSettings::IsAutoLoadingEnabled()
{
  US_UNUSED(ModuleSettingsPrivate::Lock(pimpl));
#ifdef US_ENABLE_AUTOLOADING_SUPPORT
  return !pimpl->autoLoadingDisabled &&
      pimpl->autoLoadingEnabled;
#else
  return false;
#endif
}

void ModuleSettings::SetAutoLoadingEnabled(bool enable)
{
  US_UNUSED(ModuleSettingsPrivate::Lock(pimpl));
  pimpl->autoLoadingEnabled = enable;
}

ModuleSettings::PathList ModuleSettings::GetAutoLoadPaths()
{
  US_UNUSED(ModuleSettingsPrivate::Lock(pimpl));
  ModuleSettings::PathList paths(pimpl->autoLoadPaths.begin(),
                                 pimpl->autoLoadPaths.end());
  paths.insert(paths.end(), pimpl->extraPaths.begin(),
               pimpl->extraPaths.end());
  std::sort(paths.begin(), paths.end());
  paths.erase(std::unique(paths.begin(), paths.end()), paths.end());
  return paths;
}

void ModuleSettings::SetAutoLoadPaths(const PathList& paths)
{
  PathList normalizedPaths;
  normalizedPaths.resize(paths.size());
  std::transform(paths.begin(), paths.end(), normalizedPaths.begin(), RemoveTrailingPathSeparator);

  US_UNUSED(ModuleSettingsPrivate::Lock(pimpl));
  pimpl->autoLoadPaths.clear();
  pimpl->autoLoadPaths.insert(normalizedPaths.begin(), normalizedPaths.end());
}

void ModuleSettings::AddAutoLoadPath(const std::string& path)
{
  US_UNUSED(ModuleSettingsPrivate::Lock(pimpl));
  pimpl->autoLoadPaths.insert(RemoveTrailingPathSeparator(path));
}

US_END_NAMESPACE
