/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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
#include "usStaticInit_p.h"

#include <string>
#include <set>

US_BEGIN_NAMESPACE

std::string ModuleSettings::CURRENT_MODULE_PATH()
{
  static const std::string var = "us_current_module_path";
  return var;
}

struct ModuleSettingsPrivate : public US_DEFAULT_THREADING<ModuleSettingsPrivate>
{
  ModuleSettingsPrivate()
    : autoLoadPaths()
  #ifdef US_ENABLE_AUTOLOADING_SUPPORT
    , autoLoadingEnabled(true)
  #else
    , autoLoadingEnabled(false)
  #endif
  {
    autoLoadPaths.insert(ModuleSettings::CURRENT_MODULE_PATH());
  }

  std::set<std::string> autoLoadPaths;
  bool autoLoadingEnabled;
};

US_GLOBAL_STATIC(ModuleSettingsPrivate, moduleSettingsPrivate)

bool ModuleSettings::IsThreadingSupportEnabled()
{
#ifdef US_ENABLE_THREADING_SUPPORT
  return true;
#else
  return false;
#endif
}

bool ModuleSettings::IsServiceFactorySupportEnabled()
{
#ifdef US_ENABLE_SERVICE_FACTORY_SUPPORT
  return true;
#else
  return false;
#endif
}

bool ModuleSettings::IsAutoLoadingEnabled()
{
  ModuleSettingsPrivate::Lock l(moduleSettingsPrivate());
#ifdef US_ENABLE_AUTOLOADING_SUPPORT
  return moduleSettingsPrivate()->autoLoadingEnabled;
#else
  return false;
#endif
}

void ModuleSettings::SetAutoLoadingEnabled(bool enable)
{
  ModuleSettingsPrivate::Lock l(moduleSettingsPrivate());
  moduleSettingsPrivate()->autoLoadingEnabled = enable;
}

ModuleSettings::PathList ModuleSettings::GetAutoLoadPaths()
{
  ModuleSettingsPrivate::Lock l(moduleSettingsPrivate());
  ModuleSettings::PathList paths(moduleSettingsPrivate()->autoLoadPaths.begin(),
                                 moduleSettingsPrivate()->autoLoadPaths.end());
  return paths;
}

void ModuleSettings::SetAutoLoadPaths(const PathList& paths)
{
  ModuleSettingsPrivate::Lock l(moduleSettingsPrivate());
  moduleSettingsPrivate()->autoLoadPaths.clear();
  moduleSettingsPrivate()->autoLoadPaths.insert(paths.begin(), paths.end());
}

void ModuleSettings::AddAutoLoadPath(const std::string& path)
{
  ModuleSettingsPrivate::Lock l(moduleSettingsPrivate());
  moduleSettingsPrivate()->autoLoadPaths.insert(path);
}

US_END_NAMESPACE
