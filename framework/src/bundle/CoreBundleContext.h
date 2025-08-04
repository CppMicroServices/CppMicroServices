/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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

/**

\defgroup MicroServices Micro Services Classes

\brief This category includes classes related to the C++ Micro Services component.

The C++ Micro Services component provides a dynamic service registry based on the service layer
as specified in the OSGi R4.2 specifications.

*/

/**

\defgroup MicroServicesUtils Utility Classes

\brief This category includes utility classes which can be used by others.

*/

#ifndef CPPMICROSERVICES_COREBUNDLECONTEXT_H
#define CPPMICROSERVICES_COREBUNDLECONTEXT_H

#include "cppmicroservices/Any.h"
#include "cppmicroservices/detail/Log.h"
#include "cppmicroservices/detail/Threads.h"

#include "BundleHooks.h"
#include "BundleRegistry.h"
#include "CFRLogger.h"
#include "Resolver.h"
#include "ServiceHooks.h"
#include "ServiceListeners.h"
#include "ServiceRegistry.h"

#include <map>
#include <ostream>
#include <shared_mutex>
#include <string>

namespace cppmicroservices
{
    using WriteLock = std::unique_lock<std::shared_timed_mutex>;
    using ReadLock = std::shared_lock<std::shared_timed_mutex>;

    struct FrameworkShutdownBlocker
    {
        bool frameworkHasStopped;
        ReadLock lock;

        FrameworkShutdownBlocker(bool s, ReadLock l) : frameworkHasStopped(s), lock(std::move(l)) {}
    };

    struct BundleStorage;
    class FrameworkPrivate;

    /**
     * This class is not part of the public API.
     */
    class CoreBundleContext
    {
      public:
        /**
         * Please note: The order of the member variables in this class is important. When the
         * CoreBundleContext object is destroyed, it will call the destructors for the member
         * variables in the reverse order in which they are listed here. For example, serviceHooks
         * will be destroyed before the logger. The logger will be destroyed before the listeners, etc.
         *
         * The logger has a ServiceTracker member variable. When it is destroyed, the ServiceTracker::Close
         * method is called to remove the ServiceListener. The logger object must be destroyed before the
         * listeners member variable is destroyed because when the listeners member variable is destroyed
         * it leaves the ServiceListeners data structures in an unusable state. If the logger object
         * destructor runs after the listeners object destructor, it results in an access violation.
         */
        /**
         * Framework id.
         */
        int id;

        /**
         * Id to use for next instance of the framework.
         */
        static std::atomic<int> globalId;

        /*
         * Framework properties, which contain both the
         * launch properties and the system properties.
         * See OSGi spec revision 6, section 4.2.2
         *
         * Note: CppMicroServices currently has no concept
         * of "system properties".
         */
        std::unordered_map<std::string, Any> frameworkProperties;

        std::string const& workingDir;

        /**
         * The diagnostic logging sink
         * For internal Framework use only. Do not expose
         * to Framework clients.
         */
        std::shared_ptr<detail::LogSink> sink;

        /**
         * Bundle Storage
         */
        std::unique_ptr<BundleStorage> storage;

        /**
         * Private Bundle Data Storage
         */
        std::string dataStorage;

        /**
         * All listeners in this framework.
         */
        ServiceListeners listeners;

        /**
         * All registered services in this framework.
         */
        ServiceRegistry services;

        /**
         * A LogService for logging framework messages via
         * a default or user-provided LogService that are intended to be
         * visible outside of the framework.
         */
        std::shared_ptr<cppmicroservices::cfrimpl::CFRLogger> logger;

        /**
         * All service hooks.
         */
        ServiceHooks serviceHooks;

        /**
         * All bundle hooks.
         */
        BundleHooks bundleHooks;

        /**
         * All capabilities, exported and imported packages in this framework.
         */
        Resolver resolver;

        /**
         * All installed bundles.
         */
        BundleRegistry bundleRegistry;

        bool firstInit;

        /**
         * Framework init count.
         */
        int initCount;

        std::shared_ptr<FrameworkPrivate> systemBundle;

        /**
         * Flags to use for dlopen calls on unix systems. Ignored on Windows.
         */
        int libraryLoadOptions;

        std::function<bool(cppmicroservices::Bundle const&)> validationFunc;

        ~CoreBundleContext();

        // thread-safe shared_from_this implementation
        std::shared_ptr<CoreBundleContext> shared_from_this() const;
        void SetThis(std::shared_ptr<CoreBundleContext> const& self);

        void Init();

        // must be called without any locks held
        void Uninit0();

        void Uninit1();

        /**
         * Called when framework shutdown/startup has begun.
         * This blocks (while returned object is held):
         *     - other calls to start or stop the framework
         *     - calls to start bundles
         */
        WriteLock SetFrameworkStateAndBlockUntilComplete(bool desiredState);

        /**
         * Called when bundle startup is occuring
         * This blocks (while returned object is held):
         *    - calls to start or stop the framework
         * And allows:
         *    - concurrent calls for bundle startup on other bundles
         */
        std::unique_ptr<FrameworkShutdownBlocker> GetFrameworkStateAndBlock() const;

        /**
         * Get private bundle data storage file handle.
         *
         */
        std::string GetDataStorage(long id) const;

      private:
        // The core context is exclusively constructed by the FrameworkFactory class
        friend class FrameworkFactory;

        // Mutex required to be held when changing stopped.
        // ReadLock or WriteLock construction is done using this mutex.
        mutable std::shared_timed_mutex stoppedLock;

        // Flag for whether the Framework has been stopped. See mutex stoppedLock
        bool stopped;

        /**
         * Construct a core context
         *
         */
        CoreBundleContext(std::unordered_map<std::string, Any> const& props, std::ostream* logger);

        struct : detail::MultiThreaded<>
        {
            std::weak_ptr<CoreBundleContext> v;
        } self;
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_COREBUNDLECONTEXT_H
