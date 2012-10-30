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


#ifndef USMODULESETTINGS_H
#define USMODULESETTINGS_H

#include "usConfig.h"

#include <vector>
#include <string>

US_BEGIN_NAMESPACE

/**
 * \ingroup MicroServices
 *
 * Query and set certain properties of the CppMicroServices library.
 *
 * The following environment variables influence the runtime behavior
 * of the CppMicroServices library:
 *
 * - \e US_DISABLE_AUTOLOADING If set, auto-loading of modules is disabled.
 * - \e US_AUTOLOAD_PATHS A ':' (Unix) or ';' (Windows) separated list of paths
 *   from which modules should be auto-loaded.
 *
 * \remarks This class is thread safe.
 */
class US_EXPORT ModuleSettings
{
public:

  typedef std::vector<std::string> PathList;

  /**
   * Returns a special string which can be used as an argument for a
   * AddAutoLoadPath() call.
   *
   * When a module is loaded and this string has been added as a path
   * to the list of auto-load paths the CppMicroServices library will
   * auto-load all modules from the currently being loaded module's
   * auto-load directory.
   *
   * \return A string to be used in AddAutoLoadPath().
   *
   * \remarks The returned string is contained in the default set of
   * auto-load paths, unless a new set of paths is given by a call to
   * SetAutoLoadPaths().
   *
   * \sa MicroServices_AutoLoading
   * \sa US_INITIALIZE_MODULE
   */
  static std::string CURRENT_MODULE_PATH();

  /**
   * \return \c true if threading support has been configured into the
   * CppMicroServices library, \c false otherwise.
   */
  static bool IsThreadingSupportEnabled();

  /**
   * \return \c true if support for service factories has been configured into the
   * CppMicroServices library, \c false otherwise.
   */
  static bool IsServiceFactorySupportEnabled();

  /**
   * \return \c true if support for module auto-loading is enabled,
   * \c false otherwise.
   *
   * \remarks This method will always return \c false if support for auto-loading
   * has not been configured into the CppMicroServices library or if it has been
   * disabled by defining the US_DISABLE_AUTOLOADING environment variable.
   */
  static bool IsAutoLoadingEnabled();

  /**
   * Enable or disable auto-loading support.
   *
   * \param enable If \c true, enable auto-loading support, disable it otherwise.
   *
   * \remarks Calling this method will have no effect if support for
   * auto-loading has not been configured into the CppMicroServices library of it
   * it has been disabled by defining the US_DISABLE_AUTOLOADING envrionment variable.
   */
  static void SetAutoLoadingEnabled(bool enable);

  /**
   * \return A list of paths in the file-system from which modules will be
   * auto-loaded.
   */
  static PathList GetAutoLoadPaths();

  /**
   * Set a list of paths in the file-system from which modules should be
   * auto-loaded.
   * @param paths A list of absolute file-system paths.
   */
  static void SetAutoLoadPaths(const PathList& paths);

  /**
   * Add a path in the file-system to the list of paths from which modules
   * will be auto-loaded.
   *
   * @param path The additional absolute auto-load path in the file-system.
   */
  static void AddAutoLoadPath(const std::string& path);

private:

  // purposely not implemented
  ModuleSettings();
  ModuleSettings(const ModuleSettings&);
  ModuleSettings& operator=(const ModuleSettings&);
};

US_END_NAMESPACE

#endif // USMODULESETTINGS_H
