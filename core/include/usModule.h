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


#ifndef USMODULE_H
#define USMODULE_H

#include "usModuleVersion.h"

#include <vector>

US_BEGIN_NAMESPACE

class Any;
class CoreModuleContext;
struct ModuleInfo;
class ModuleContext;
class ModuleResource;
class ModulePrivate;

template<class S>
class ServiceReference;

typedef ServiceReference<void> ServiceReferenceU;

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
 * A module can be in one of four states:
 * <ul>
 * <li>INSTALLED
 * <li>LOADED
 * <li>UNLOADED
 * <li>UNINSTALLED
 * </ul>
 * <p>
 * You can determine whether a bundle is loaded or not by using IsLoaded().
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
class US_Core_EXPORT Module
{

public:

  Module(const Module &) = delete;
  Module& operator=(const Module&) = delete;

  /**
   * Returns the property key for looking up this module's id.
   * The property value is of type \c long.
   *
   * @return The id property key.
   */
  static const std::string& PROP_ID();

  /**
   * Returns the property key for looking up this module's name.
   * The property value is of type \c std::string.
   *
   * @return The name property key.
   */
  static const std::string& PROP_NAME();

  /**
   * Returns the property key for looking up this module's
   * location in the file system.
   * The property value is of type \c std::string.
   *
   * @return The location property key.
   */
  static const std::string& PROP_LOCATION();

  /**
   * Returns the property key with a value of \c module.version for looking
   * up this module's version identifier.
   * The property value is of type \c std::string.
   *
   * @return The version property key.
   */
  static const std::string& PROP_VERSION();

  /**
   * Returns the property key with a value of \c module.vendor for looking
   * up this module's vendor information.
   * The property value is of type \c std::string.
   *
   * @return The vendor property key.
   */
  static const std::string& PROP_VENDOR();

  /**
   * Returns the property key with a value of \c module.description for looking
   * up this module's description.
   * The property value is of type \c std::string.
   *
   * @return The description property key.
   */
  static const std::string& PROP_DESCRIPTION();

  /**
   * Returns the property key with a value of \c module.autoload_dir for looking
   * up this module's auto-load directory.
   * The property value is of type \c std::string.
   *
   * @return The auto-load directory property key.
   */
  static const std::string& PROP_AUTOLOAD_DIR();

  /**
   * Returns the property key with a value of \c module.autoloaded_modules for
   * looking up this module's auto-load modules.
   * The property value is of type \c std::vector<std::string> and contains
   * the file system locations for the auto-loaded modules triggered by this
   * module.
   *
   * @return The auto-loaded modules property key.
   */
  static const std::string& PROP_AUTOLOADED_MODULES();

  virtual ~Module();

  /**
   * Returns this module's current state.
   *
   * <p>
   * A module can be in only one state at any time.
   *
   * @return <code>true</code> if the module is <code>LOADED</code>
   *         <code>false</code> if it is in any other state.
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
  virtual std::string GetLocation() const;

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
   * Returns the value of the specified property for this module.
   * If not found, the framework's properties are searched. 
   * The method returns an empty Any if the property is not found.
   *
   * @param key The name of the requested property.
   * @return The value of the requested property, or an empty string
   *         if the property is undefined.
   *
   * @sa GetPropertyKeys()
   * @sa \ref MicroServices_ModuleProperties
   */
  Any GetProperty(const std::string& key) const;

  /**
   * Returns a list of top-level property keys for this module.
   *
   * @return A list of available property keys.
   *
   * @sa \ref MicroServices_ModuleProperties
   */
  std::vector<std::string> GetPropertyKeys() const;

  /**
   * Returns this module's ServiceReference list for all services it
   * has registered or an empty list if this module has no registered
   * services.
   *
   * The list is valid at the time of the call to this method, however,
   * as the framework is a very dynamic environment, services can be
   * modified or unregistered at anytime.
   *
   * @return A list of ServiceReference objects for services this
   * module has registered.
   */
  std::vector<ServiceReferenceU> GetRegisteredServices() const;

  /**
   * Returns this module's ServiceReference list for all services it is
   * using or returns an empty list if this module is not using any
   * services. A module is considered to be using a service if its use
   * count for that service is greater than zero.
   *
   * The list is valid at the time of the call to this method, however,
   * as the framework is a very dynamic environment, services can be
   * modified or unregistered at anytime.
   *
   * @return A list of ServiceReference objects for all services this
   * module is using.
   */
  std::vector<ServiceReferenceU> GetServicesInUse() const;

  /**
   * Returns the resource at the specified \c path in this module.
   * The specified \c path is always relative to the root of this module and may
   * begin with '/'. A path value of "/" indicates the root of this module.
   *
   * @param path The path name of the resource.
   * @return A ModuleResource object for the given \c path. If the \c path cannot
   * be found in this module or the module's state is \c UNLOADED, an invalid
   * ModuleResource object is returned.
   */
  ModuleResource GetResource(const std::string& path) const;

  /**
   * Returns resources in this module.
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
   * @param path The path name in which to look. The path is always relative to the root
   * of this module and may begin with '/'. A path value of "/" indicates the root of this module.
   * @param filePattern The resource name pattern for selecting entries in the specified path.
   * The pattern is only matched against the last element of the resource path. Substring
   * matching is supported using the wildcard charachter ('*'). If \c filePattern is empty,
   * this is equivalent to "*" and matches all resources.
   * @param recurse If \c true, recurse into subdirectories. Otherwise only return resources
   * from the specified path.
   * @return A vector of ModuleResource objects for each matching entry.
   */
  std::vector<ModuleResource> FindResources(const std::string& path, const std::string& filePattern, bool recurse) const;

  /**
   * Start this bundle.
   * 
   * The following steps are required to start this bundle:
   * -# If this bundle is in the process of being activated or deactivated then this method must wait for
   *    activation or deactivation to complete before continuing. If this does not occur in a reasonable
   *    time, a std::runtime_error exception is thrown to indicate this bundle was unable to be started.
   * -# If this bundle was already started, then this method returns immediately.
   * -# A bundle event of type BundleEvent::STARTING is fired.
   * -# The BundleActivator::Start(BundleContext) method of this bundle's BundleActivator, if one is
   *    specified, is called. If the BundleActivator is invalid or throws an exception then:
   *    - A bundle event of type BundleEvent::STOPPING is fired.
   *    - %Any services registered by this bundle must be unregistered.
   *    - %Any services used by this bundle must be released.
   *    - %Any listeners registered by this bundle must be removed.
   *    - A bundle event of type BundleEvent::STOPPED is fired.
   *    - A std::runtime_error exception is then thrown.
   * -# A bundle event of type BundleEvent::STARTED is fired.
   *
   * @throws std::runtime_error If this bundle could not be started.
   */
  virtual void Start();

  /**
   * Stop this bundle.
   *
   * The following steps are required to stop a bundle:
   * -# If this bundle is in the process of being activated or deactivated then this method must wait for
   *    activation or deactivation to complete before continuing. If this does not occur in a reasonable
   *    time, a std::runtime_error exception is thrown to indicate this bundle was unable to be stopped.
   * -# If this bundle was already stopped, then this method returns immediately.
   * -# A bundle event of type BundleEvent::STOPPING is fired.
   * -# The BundleActivator::Stop(BundleContext) method of this bundle's BundleActivator, if one is specified,
   *    is called. If that method throws an exception, this method must continue to stop this bundle
   *    and a std::runtime_error exception must be thrown after completion of the remaining steps.
   * -# %Any services registered by this bundle must be unregistered.
   * -# %Any services used by this bundle must be released.
   * -# %Any listeners registered by this bundle must be removed.
   * -# A bundle event of type BundleEvent::STOPPED is fired.
   *
   * @throws std::runtime_error If the bundle failed to stop.
   */
  virtual void Stop();

  /**
   * Uninstalls this bundle.
   * 
   * This method causes the Framework to notify other bundles that this bundle is being uninstalled,
   * and then uninstalls this bundle. The Framework must remove any resources related to this bundle 
   * that it is able to remove.
   *
   * The following steps are required to uninstall a bundle:
   * -# This bundle is stopped as described in the Bundle.Stop method.
   * -# A bundle event of BundleEvent::UNINSTALLED is fired.
   * -# This bundle and any persistent storage area provided for this bundle by the Framework are removed.
   *
   * @throws std::runtime_error If the bundle could not be uninstalled.   *
   */
  virtual void Uninstall();

private:

  friend class CoreModuleActivator;
  friend class ModuleRegistry;
  friend class ServiceReferencePrivate;
  friend class Framework;

  ModulePrivate* d;

  Module();

  void Init(CoreModuleContext* coreCtx, ModuleInfo* info);
  void Uninit();

};

US_END_NAMESPACE

/**
 * \ingroup MicroServices
 */
US_Core_EXPORT std::ostream& operator<<(std::ostream& os, const US_PREPEND_NAMESPACE(Module)& module);
/**
 * \ingroup MicroServices
 */
US_Core_EXPORT std::ostream& operator<<(std::ostream& os, US_PREPEND_NAMESPACE(Module) const * module);

#endif // USMODULE_H
