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

#ifndef USFRAMEWORK_H
#define USFRAMEWORK_H

#include <usCoreConfig.h>
#include "usBundle.h"

#include <map>
#include <string>

namespace us {

class FrameworkPrivate;

/**
 * \ingroup MicroServices
 *
 * The CppMicroServices Framework.
 *
 * <p>
 * A Framework is itself a bundle and is known as the "System Bundle".
 * The System Bundle differs from other bundles in the following ways:
 * - The system bundle is always assigned a bundle identifier of zero (0).
 * - The system bundle <code>GetLocation</code> method returns the string: "System Bundle".
 * - The system bundle's life cycle cannot be managed like normal bundles. Its life cycle methods
 *   must behave as follows:
 *   - Start - Does nothing because the system bundle is already started.
 *   - Stop - Stops all installed bundles.
 *   - Uninstall - The Framework must throw a std::runtime_error exception indicating that the
 *     system bundle cannot be uninstalled.
 *
 * Framework instances are created using a FrameworkFactory. The methods of this class can be
 * used to manage and control the created framework instance.
 *
 * @remarks This class is thread-safe.
 *
 * @see FrameworkFactory::NewFramework(std::map<std::string, std::string> configuration)
 */
class US_Core_EXPORT Framework : public Bundle
{
public:

    // This class is not copy-able
    Framework(const Framework&) = delete;
    Framework operator=(const Framework&) = delete;

    virtual ~Framework(void);

    /**
     * Start this Framework.
     *
     * <p>
     * The following steps are taken to start this Framework:
     * -# If this Framework is not in the STARTING state, start this Framework.
     * -# This Framework's state is set to ACTIVE.
     *
     * @throws std::runtime_error If this Framework could not be started.
     */
    void Start();

    /**
     * Stop this Framework.
     *
     * The following steps are taken to stop this Framework:
     * -# This Framework's state is set to STOPPING.
     * -# All installed bundles must be stopped.
     * -# Unregister all services registered by this Framework.
     * -# Event handling is disabled.
     * -# All resources held by this Framework are released. This includes threads, open files, etc.
     *
     * After being stopped, this Framework may be discarded or started.
     *
     * @throws std::runtime_error If stopping this Framework could not be initiated.
     */
    void Stop();

    /**
     * The Framework cannot be uninstalled.
     *
     * This method always throws a std::runtime_error exception.
     *
     * @throws std::runtime_error This Framework cannot be uninstalled.
     */
    void Uninstall();

    /**
    * Returns this Framework's location.
    *
    * <p>
    * This Framework is assigned the unique location "System Bundle"
    * since this Framework is also a System Bundle.
    *
    * @return The string "System Bundle".
    */
    std::string GetLocation() const;

    /**
     * Enable or disable auto-install support.
     *
     * @param enable If \c true, enable auto-install support, disable it otherwise.
     *
     * @remarks Calling this method will have no effect if support for
     * auto-loading has not been configured into the CppMicroServices library or if
     * it has been disabled by defining the US_DISABLE_AUTOLOADING envrionment variable.
     *
     * @deprecated This method remains for legacy clients and will be removed in a
     * future release. Auto-loading support will be moved into a seprate service,
     * at which point, this method will be permanently removed.
     *
     */
    void SetAutoLoadingEnabled(bool enable);

    /**
     * The framework's persistent storage base path property key name.
     * This property's default value is the current working directory.
     *
     */
    static const std::string PROP_STORAGE_LOCATION;

    /**
     * The framework's threading support property key name.
     * This property's default value is "single".
     * Valid key values are:
     * - "single" - The framework APIs are not thread-safe.
     * - "multi" - The framework APIs are thread-safe.
     *
     * @remarks This is a read-only property and cannot be altered at run-time.
     * The key's value is set at compile time by the US_ENABLE_THREADING_SUPPORT option.
     *
     * @see \ref BuildInstructions
     */
    static const std::string PROP_THREADING_SUPPORT;

    /**
     * The framework's log level property key name.
     * This property's default value is "3" (Only errors are logged).
     *
     * @see MsgType
     */
    static const std::string PROP_LOG_LEVEL;

private:
    // Framework instances are exclusively constructed by the FrameworkFactory class
    friend class FrameworkFactory;

    // Allow the framework to be constructed with configuration properties
    // provided by a FrameworkFactory object.
    Framework(const BundleInfo& info, const std::map<std::string, Any>& configuration);

};

}

#endif
