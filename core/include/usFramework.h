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

#include "usModule.h"

#include <map>

class CoreModuleContext;

US_BEGIN_NAMESPACE

/**
 * A Framework instance. A Framework is itself a bundle and is known as the "System Bundle".
 *
 * The System Bundle differs from other bundles in the following ways:
 * - The system bundle is always assigned a bundle identifier of zero (0).
 * - The system bundle getLocation method returns the string: "System Bundle", as defined in the
 *   Constants interface.
 * - The system bundle has a bundle symbolic name that is unique for a specific version. However,
 *   the name system.bundle must be recognized as an alias to this implementation-defined name.
 * - The system bundle's life cycle cannot be managed like normal bundles. Its life cycle methods
 *   must behave as follows:
 *   - start - Does nothing because the system bundle is already started.
 *   - stop - Returns immediately and shuts down the Framework on another thread.
 *   - update - Returns immediately, then stops and restarts the Framework on another thread.
 *   - uninstall - The Framework must throw a BundleException indicating that the system bundle 
 *     cannot be uninstalled.
 *
 * Framework instances are created using a FrameworkFactory. The methods of this interface can be
 * used to manage and control the created framework instance.
 */
class US_Core_EXPORT Framework : public Module
{
public:
    Framework(void);
    ~Framework(void);

    /**
     * Initialize this Framework.
     *
     * \throw BundleException If this Framework could not be initialized.
     */
    void init(void);

    /**
     * \param timeout Maximum number of milliseconds to wait until this Framework has completely stopped. A value of
     * zero will wait indefinitely.
     * 
     * \remarks Wait until this Framework has completely stopped. The stop and update methods on a Framework performs an 
     * asynchronous stop of the Framework. This method can be used to wait until the asynchronous stop of this 
     * Framework has completed. This method will only wait if called when this Framework is in the 
     * STARTING, ACTIVE, or STOPPING states. Otherwise it will return immediately. A Framework Event is returned 
     * to indicate why this Framework has stopped.
     *
     * \return A Framework Event indicating the reason this method returned. The following FrameworkEvent types 
     * may be returned by this method.
     * - STOPPED - This Framework has been stopped.
     * - STOPPED_UPDATE - This Framework has been updated which has shutdown and will now restart.
     * - ERROR - The Framework encountered an error while shutting down or an error has occurred which forced the framework to shutdown.
     * - WAIT_TIMEDOUT - This method has timed out and returned before this Framework has stopped.
     * 
     * \throw InterruptedException If another thread interrupted the current thread before or while the current
     *  thread was waiting for this Framework to completely stop. The interrupted status of the current thread 
     *  is cleared when this exception is thrown.
     * \throw IllegalArgumentException If the value of timeout is negative.
     */
    void waitForStop(void) { /* TODO: implement */ }

    void Start();
    void Stop() { /* TODO: implement */ }
    void Update() { /* TODO: implement */ }
    void Uninstall();

    std::string GetLocation() const;

    /****************************************************
     * BEGIN - Non OSGi compliant functions
     *
     * Auto-load/install will be factored out into a separate
     * service.
     ****************************************************/

    /**
     * Enable or disable auto-loading support.
     *
     * \param enable If \c true, enable auto-loading support, disable it otherwise.
     *
     * \remarks Calling this method will have no effect if support for
     * auto-loading has not been configured into the CppMicroServices library of it
     * it has been disabled by defining the US_DISABLE_AUTOLOADING envrionment variable.
     */
    void SetAutoLoadingEnabled(bool enable);

    /********************************************
     * END - Non OSGi compliant functions
     ********************************************/

private:

    friend class FrameworkFactory;
    
    // Allow the framework to be constructed with configuration properties
    // provided by a FrameworkFactory object.
    Framework(std::map<std::string, std::string>& configuration);

    // This class is not copy-able
    Framework(const Framework&);
    Framework operator=(const Framework&);

    CoreModuleContext* coreModuleContext;

    // contains the launching properties (OSGi section 4.2.2)
    std::map<std::string, std::string> launchProperties;
    // TODO: make thread-safe
    bool initialized;
};

US_END_NAMESPACE

#endif
