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


#ifndef USBUNDLE_H
#define USBUNDLE_H

#include "usBundleVersion.h"

#include <vector>
#include <memory>

namespace us {

class Any;
class CoreBundleContext;
struct BundleInfo;
class BundleContext;
class BundleResource;
class BundlePrivate;

template<class S>
class ServiceReference;

typedef ServiceReference<void> ServiceReferenceU;

/**
 * \ingroup MicroServices
 *
 * Represents a CppMicroServices bundle.
 *
 * <p>
 * A <code>%Bundle</code> object is the access point to a CppMicroServices bundle.
 * Each CppMicroServices bundle has an associated <code>%Bundle</code> object.
 *
 * <p>
 * A bundle has unique identity, a <code>long</code>, chosen by the
 * framework. This identity does not change during the lifecycle of a bundle.
 *
 * <p>
 * A bundle can be in one of four states:
 * <ul>
 * <li>INSTALLED
 * <li>STARTED
 * <li>STOPPED
 * <li>UNINSTALLED
 * </ul>
 * <p>
 * You can determine whether a bundle is started or not by using IsStarted().
 *
 * <p>
 * A bundle can only execute code when its state is <code>STARTED</code>.
 * A <code>STOPPED</code> bundle is a
 * zombie and can only be reached because it was started before. However,
 * stopped bundles can be started again.
 *
 * <p>
 * The framework is the only entity that is allowed to create
 * <code>%Bundle</code> objects.
 *
 * @remarks This class is thread safe.
 */
class US_Core_EXPORT Bundle : public std::enable_shared_from_this<Bundle>
{

public:

  Bundle(const Bundle &) = delete;
  Bundle& operator=(const Bundle&) = delete;

  /**
   * The property key for looking up this bundle's id.
   * The property value is of type \c long.
   *
   * @see \ref MicroServices_BundleProperties
   */
  static const std::string PROP_ID;

  /**
   * The property key for looking up this bundle's name.
   * The property value is of type \c std::string.
   *
   * @see \ref MicroServices_BundleProperties
   */
  static const std::string PROP_NAME;

  /**
   * The property key for looking up this bundle's location in the file system.
   * The property value is of type \c std::string.
   *
   * @see \ref MicroServices_BundleProperties
   */
  static const std::string PROP_LOCATION;

  /**
   * The property key for looking up this bundle's version identifier.
   * The property value is of type \c std::string.
   *
   * @see \ref MicroServices_BundleProperties
   */
  static const std::string PROP_VERSION;

  /**
   * The property key for looking up this bundle's vendor information.
   * The property value is of type \c std::string.
   *
   * @see \ref MicroServices_BundleProperties
   */
  static const std::string PROP_VENDOR;

  /**
   * The property key for looking up this bundle's description.
   * The property value is of type \c std::string.
   *
   * @see \ref MicroServices_BundleProperties
   */
  static const std::string PROP_DESCRIPTION;

  virtual ~Bundle();

  /**
   * Returns this bundle's current state.
   *
   * <p>
   * A bundle can be in only one state at any time.
   *
   * @return <code>true</code> if the bundle is <code>STARTED</code>
   *         <code>false</code> if it is in any other state.
   */
  bool IsStarted() const;

  /**
   * Returns this bundle's {@link BundleContext}. The returned
   * <code>BundleContext</code> can be used by the caller to act on behalf
   * of this bundle.
   *
   * <p>
   * If this bundle is not in the <code>STARTED</code> state, then this
   * bundle has no valid <code>BundleContext</code>. This method will
   * return <code>nullptr</code> if this bundle has no valid
   * <code>BundleContext</code>.
   *
   * @return A <code>BundleContext</code> for this bundle or
   *         <code>0</code> if this bundle has no valid
   *         <code>BundleContext</code>.
   */
  BundleContext* GetBundleContext() const;

  /**
   * Returns this bundle's unique identifier. This bundle is assigned a unique
   * identifier by the framework when it was installed.
   *
   * <p>
   * A bundle's unique identifier has the following attributes:
   * <ul>
   * <li>Is unique.
   * <li>Is a <code>long</code>.
   * <li>Its value is not reused for another bundle, even after a bundle is
   * uninstalled.
   * <li>Does not change while a bundle remains installed.
   * <li>Does not change when a bundle is re-started.
   * </ul>
   *
   * <p>
   * This method continues to return this bundle's unique identifier while
   * this bundle is in the <code>STOPPED</code> state.
   *
   * @return The unique identifier of this bundle.
   */
  long GetBundleId() const;

  /**
   * Returns this bundle's location.
   *
   * <p>
   * The location is the full path to the bundle's shared library.
   * This method continues to return this bundle's location
   * while this bundle is in the <code>STOPPED</code> state.
   *
   * @return The string representation of this bundle's location.
   */
  virtual std::string GetLocation() const;

  /**
   * Returns the name of this bundle as specified by the
   * US_CREATE_BUNDLE CMake macro. The bundle
   * name together with a version must identify a unique bundle.
   *
   * <p>
   * This method continues to return this bundle's name while
   * this bundle is in the <code>STOPPED</code> state.
   *
   * @return The name of this bundle.
   */
  std::string GetName() const;

  /**
   * Returns the version of this bundle as specified by the
   * US_INITIALIZE_BUNDLE CMake macro. If this bundle does not have a
   * specified version then {@link BundleVersion::EmptyVersion} is returned.
   *
   * <p>
   * This method continues to return this bundle's version while
   * this bundle is in the <code>STOPPED</code> state.
   *
   * @return The version of this bundle.
   */
  BundleVersion GetVersion() const;

  /**
   * Returns the value of the specified property for this bundle.
   * If not found, the framework's properties are searched.
   * The method returns an empty Any if the property is not found.
   *
   * @param key The name of the requested property.
   * @return The value of the requested property, or an empty string
   *         if the property is undefined.
   *
   * @sa GetPropertyKeys()
   * @sa \ref MicroServices_BundleProperties
   */
  Any GetProperty(const std::string& key) const;

  /**
   * Returns a list of top-level property keys for this bundle.
   *
   * @return A list of available property keys.
   *
   * @sa \ref MicroServices_BundleProperties
   */
  std::vector<std::string> GetPropertyKeys() const;

  /**
   * Returns this bundle's ServiceReference list for all services it
   * has registered or an empty list if this bundle has no registered
   * services.
   *
   * The list is valid at the time of the call to this method, however,
   * as the framework is a very dynamic environment, services can be
   * modified or unregistered at anytime.
   *
   * @return A list of ServiceReference objects for services this
   * bundle has registered.
   */
  std::vector<ServiceReferenceU> GetRegisteredServices() const;

  /**
   * Returns this bundle's ServiceReference list for all services it is
   * using or returns an empty list if this bundle is not using any
   * services. A bundle is considered to be using a service if its use
   * count for that service is greater than zero.
   *
   * The list is valid at the time of the call to this method, however,
   * as the framework is a very dynamic environment, services can be
   * modified or unregistered at anytime.
   *
   * @return A list of ServiceReference objects for all services this
   * bundle is using.
   */
  std::vector<ServiceReferenceU> GetServicesInUse() const;

  /**
   * Returns the resource at the specified \c path in this bundle.
   * The specified \c path is always relative to the root of this bundle and may
   * begin with '/'. A path value of "/" indicates the root of this bundle.
   *
   * @param path The path name of the resource.
   * @return A BundleResource object for the given \c path. If the \c path cannot
   * be found in this bundle or the bundle's state is \c STOPPED, an invalid
   * BundleResource object is returned.
   */
  BundleResource GetResource(const std::string& path) const;

  /**
   * Returns resources in this bundle.
   *
   * This method is intended to be used to obtain configuration, setup, localization
   * and other information from this bundle.
   *
   * This method can either return only resources in the specified \c path or recurse
   * into subdirectories returning resources in the directory tree beginning at the
   * specified path.
   *
   * Examples:
   * \snippet uServices-resources/main.cpp 0
   *
   * @param path The path name in which to look. The path is always relative to the root
   * of this bundle and may begin with '/'. A path value of "/" indicates the root of this bundle.
   * @param filePattern The resource name pattern for selecting entries in the specified path.
   * The pattern is only matched against the last element of the resource path. Substring
   * matching is supported using the wildcard charachter ('*'). If \c filePattern is empty,
   * this is equivalent to "*" and matches all resources.
   * @param recurse If \c true, recurse into subdirectories. Otherwise only return resources
   * from the specified path.
   * @return A vector of BundleResource objects for each matching entry.
   */
  std::vector<BundleResource> FindResources(const std::string& path, const std::string& filePattern, bool recurse) const;

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
   * @throws std::runtime_error If the bundle could not be uninstalled.
   */
  virtual void Uninstall();

protected:

  Bundle(std::unique_ptr<BundlePrivate> d);

  std::unique_ptr<BundlePrivate> d;

private:

  friend class CoreBundleActivator;
  friend class BundleRegistry;
  friend class ServiceReferencePrivate;

  Bundle(CoreBundleContext* coreCtx, const BundleInfo& info);

  void Uninit_unlocked();

};

/**
 * \ingroup MicroServices
 */
US_Core_EXPORT std::ostream& operator<<(std::ostream& os, const Bundle& bundle);
/**
 * \ingroup MicroServices
 */
US_Core_EXPORT std::ostream& operator<<(std::ostream& os, Bundle const * bundle);

}

#endif // USBUNDLE_H
