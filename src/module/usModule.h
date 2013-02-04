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


#ifndef USMODULE_H
#define USMODULE_H

#include "usModuleVersion.h"

#include <vector>

US_BEGIN_NAMESPACE

class CoreModuleContext;
struct ModuleInfo;
class ModuleContext;
class ModuleResource;
class ModulePrivate;

/**
 * \ingroup MicroServices
 *
 * Represents a CppMicroServices module.
 *
 * <p>
 * A <code>%Module</code> object is the access point to a CppMicroServices module.
 * Each CppMicroServices module has an associated <code>%Module</code> object.
 *
 * <p>
 * A module has unique identity, a <code>long</code>, chosen by the
 * framework. This identity does not change during the lifecycle of a module.
 *
 * <p>
 * A module can be in one of two states:
 * <ul>
 * <li>LOADED
 * <li>UNLOADED
 * </ul>
 * <p>
 * You can determine the current state by using IsLoaded().
 *
 * <p>
 * A module can only execute code when its state is <code>LOADED</code>.
 * An <code>UNLOADED</code> module is a
 * zombie and can only be reached because it was loaded before. However,
 * unloaded modules can be loaded again.
 *
 * <p>
 * The framework is the only entity that is allowed to create
 * <code>%Module</code> objects.
 *
 * @remarks This class is thread safe.
 */
class US_EXPORT Module
{

public:

  static const std::string& PROP_ID();
  static const std::string& PROP_NAME();
  static const std::string& PROP_LOCATION();
  static const std::string& PROP_MODULE_DEPENDS();
  static const std::string& PROP_LIB_DEPENDS();
  static const std::string& PROP_VERSION();

  ~Module();

  /**
   * Returns this module's current state.
   *
   * <p>
   * A module can be in only one state at any time.
   *
   * @return <code>true</code> if the module is <code>LOADED</code>
   *         <code>false</code> if it is <code>UNLOADED</code>
   */
  bool IsLoaded() const;

  /**
   * Returns this module's {@link ModuleContext}. The returned
   * <code>ModuleContext</code> can be used by the caller to act on behalf
   * of this module.
   *
   * <p>
   * If this module is not in the <code>LOADED</code> state, then this
   * module has no valid <code>ModuleContext</code>. This method will
   * return <code>0</code> if this module has no valid
   * <code>ModuleContext</code>.
   *
   * @return A <code>ModuleContext</code> for this module or
   *         <code>0</code> if this module has no valid
   *         <code>ModuleContext</code>.
   */
  ModuleContext* GetModuleContext() const;

  /**
   * Returns this module's unique identifier. This module is assigned a unique
   * identifier by the framework when it was loaded.
   *
   * <p>
   * A module's unique identifier has the following attributes:
   * <ul>
   * <li>Is unique.
   * <li>Is a <code>long</code>.
   * <li>Its value is not reused for another module, even after a module is
   * unloaded.
   * <li>Does not change while a module remains loaded.
   * <li>Does not change when a module is reloaded.
   * </ul>
   *
   * <p>
   * This method continues to return this module's unique identifier while
   * this module is in the <code>UNLOADED</code> state.
   *
   * @return The unique identifier of this module.
   */
  long GetModuleId() const;

  /**
   * Returns this module's location.
   *
   * <p>
   * The location is the full path to the module's shared library.
   * This method continues to return this module's location
   * while this module is in the <code>UNLOADED</code> state.
   *
   * @return The string representation of this module's location.
   */
  std::string GetLocation() const;

  /**
   * Returns the name of this module as specified by the
   * US_CREATE_MODULE CMake macro. The module
   * name together with a version must identify a unique module.
   *
   * <p>
   * This method continues to return this module's name while
   * this module is in the <code>UNLOADED</code> state.
   *
   * @return The name of this module.
   */
  std::string GetName() const;

  /**
   * Returns the version of this module as specified by the
   * US_INITIALIZE_MODULE CMake macro. If this module does not have a
   * specified version then {@link ModuleVersion::EmptyVersion} is returned.
   *
   * <p>
   * This method continues to return this module's version while
   * this module is in the <code>UNLOADED</code> state.
   *
   * @return The version of this module.
   */
  ModuleVersion GetVersion() const;

  /**
   * Returns the value of the specified property for this module. The
   * method returns an empty string if the property is not found.
   *
   * @param key The name of the requested property.
   * @return The value of the requested property, or an empty string
   *         if the property is undefined.
   */
  std::string GetProperty(const std::string& key) const;

  /**
   * Returns the resource at the specified \c path in this module.
   * The specified \c path is always relative to the root of this module and may
   * begin with '/'. A path value of "/" indicates the root of this module.
   *
   * \note In case of other modules being statically linked into this module,
   * the \c path can be ambiguous and returns the first resource matching the
   * provided \c path according to the order of the static module names in the
   * #US_LOAD_IMPORTED_MODULES macro.
   *
   * @param path The path name of the resource.
   * @return A ModuleResource object for the given \c path. If the \c path cannot
   * be found in this module or the module's state is \c UNLOADED, an invalid
   * ModuleResource object is returned.
   */
  ModuleResource GetResource(const std::string& path) const;

  /**
   * Returns resources in this module and its statically linked modules.
   *
   * This method is intended to be used to obtain configuration, setup, localization
   * and other information from this module.
   *
   * This method can either return only resources in the specified \c path or recurse
   * into subdirectories returning resources in the directory tree beginning at the
   * specified path.
   *
   * Examples:
   * \snippet uServices-resources/main.cpp 0
   *
   * \note In case of modules statically linked into this module, the returned
   * ModuleResource objects can represent the same resource path, coming from
   * different static modules. The order of the ModuleResource objects in the
   * returned container matches the order of the static module names in the
   * #US_LOAD_IMPORTED_MODULES macro.
   *
   * @param path The path name in which to look. The path is always relative to the root
   * of this module and may begin with '/'. A path value of "/" indicates the root of this module.
   * @param filePattern The resource name pattern for selecting entries in the specified path.
   * The pattern is only matched against the last element of the resource path. Substring
   * matching is supported using the wildcard charachter ('*'). If \c filePattern is empty,
   * this is equivalent to "*" and matches all resources.
   * @param recurse If \c true, recurse into subdirectories. Otherwise only return resources
   * from the specified path.
   * @return A vector of ModuleResource objects for each matching entry. The objects are sorted
   * such that resources from this module are returned first followed by the resources from
   * statically linked modules in the load order as specified in #US_LOAD_IMPORTED_MODULES.
   */
  std::vector<ModuleResource> FindResources(const std::string& path, const std::string& filePattern, bool recurse) const;

private:

  friend class ModuleRegistry;
  friend class ServiceReferencePrivate;

  ModulePrivate* d;

  Module();

  void Init(CoreModuleContext* coreCtx, ModuleInfo* info);
  void Uninit();

  void Start();
  void Stop();

  // purposely not implemented
  Module(const Module &);
  Module& operator=(const Module&);

};

US_END_NAMESPACE

/**
 * \ingroup MicroServices
 */
US_EXPORT std::ostream& operator<<(std::ostream& os, const US_PREPEND_NAMESPACE(Module)& module);
/**
 * \ingroup MicroServices
 */
US_EXPORT std::ostream& operator<<(std::ostream& os, US_PREPEND_NAMESPACE(Module) const * module);

#endif // USMODULE_H
