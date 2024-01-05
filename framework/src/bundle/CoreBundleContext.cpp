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

#include "cppmicroservices/GlobalConfig.h"

US_MSVC_DISABLE_WARNING(4355)

#include "CoreBundleContext.h"

#include "cppmicroservices/BundleInitialization.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "cppmicroservices/util/FileSystem.h"
#include "cppmicroservices/util/String.h"

#include "BundleContextPrivate.h"
#include "BundleStorageMemory.h"
#include "FrameworkPrivate.h"

#include <iomanip>
#include <memory>

#ifdef US_PLATFORM_POSIX
#    include <dlfcn.h>
#endif

CPPMICROSERVICES_INITIALIZE_BUNDLE

namespace cppmicroservices
{

    std::atomic<int> CoreBundleContext::globalId { 0 };

    std::unordered_map<std::string, Any>
    InitProperties(std::unordered_map<std::string, Any> configuration)
    {
        // Framework internal diagnostic logging is off by default
        configuration.emplace(std::make_pair(Constants::FRAMEWORK_LOG, Any(false)));

        // Framework::PROP_THREADING_SUPPORT is a read-only property whose value is based off of a compile-time switch.
        // Run-time modification of the property should be ignored as it is irrelevant.
#ifdef US_ENABLE_THREADING_SUPPORT
        configuration[Constants::FRAMEWORK_THREADING_SUPPORT] = std::string("multi");
#else
        configuration[Constants::FRAMEWORK_THREADING_SUPPORT] = std::string("single");
#endif

        configuration.emplace(std::make_pair(Constants::FRAMEWORK_WORKING_DIR, util::GetCurrentWorkingDirectory()));

        configuration.emplace(std::make_pair(Constants::FRAMEWORK_STORAGE, Any(FWDIR_DEFAULT)));

        configuration[Constants::FRAMEWORK_VERSION] = std::string(CppMicroServices_VERSION_STR);
        configuration[Constants::FRAMEWORK_VENDOR] = std::string("CppMicroServices");

#ifdef US_PLATFORM_POSIX
        configuration.emplace(std::make_pair(Constants::LIBRARY_LOAD_OPTIONS, RTLD_LAZY | RTLD_LOCAL));
#else
        configuration[Constants::LIBRARY_LOAD_OPTIONS] = int(0);
#endif

        return configuration;
    }

    CoreBundleContext::CoreBundleContext(std::unordered_map<std::string, Any> const& props, std::ostream* diagLogger)
        : id(globalId++)
        , frameworkProperties(InitProperties(props))
        , workingDir(ref_any_cast<std::string>(frameworkProperties.at(Constants::FRAMEWORK_WORKING_DIR)))
        , listeners(this)
        , services(this)
        , logger(std::make_shared<cppmicroservices::cfrimpl::CFRLogger>())
        , serviceHooks(this)
        , bundleHooks(this)
        , bundleRegistry(this)
        , firstInit(true)
        , initCount(0)
        , libraryLoadOptions(0)
        , stopped(false)
    {
        auto enableDiagLog = any_cast<bool>(frameworkProperties.at(Constants::FRAMEWORK_LOG));
        std::ostream* diagnosticLogger = (diagLogger) ? diagLogger : &std::clog;
        sink = std::make_shared<detail::LogSink>(diagnosticLogger, enableDiagLog);
        systemBundle = std::shared_ptr<FrameworkPrivate>(new FrameworkPrivate(this));
        DIAG_LOG(*sink) << "created";
    }

    CoreBundleContext::~CoreBundleContext() = default;

    std::shared_ptr<CoreBundleContext>
    CoreBundleContext::shared_from_this() const
    {
        return self.Lock(), self.v.lock();
    }

    void
    CoreBundleContext::SetThis(std::shared_ptr<CoreBundleContext> const& self)
    {
        this->self.Lock(), this->self.v = self;
    }

    void
    CoreBundleContext::Init()
    {
        DIAG_LOG(*sink) << "initializing";
        initCount++;

        auto storageCleanProp = frameworkProperties.find(Constants::FRAMEWORK_STORAGE_CLEAN);
        if (firstInit && storageCleanProp != frameworkProperties.end()
            && storageCleanProp->second == Constants::FRAMEWORK_STORAGE_CLEAN_ONFIRSTINIT)
        {
            // DeleteFWDir();
            firstInit = false;
        }

        // We use a "pseudo" random UUID.
        std::string const sid_base = "04f4f884-31bb-45c0-b176-";
        std::stringstream ss;
        ss << sid_base << std::setfill('0') << std::setw(8) << std::hex << static_cast<int32_t>(id * 65536 + initCount);

        frameworkProperties[Constants::FRAMEWORK_UUID] = ss.str();

        // $TODO we only support non-persistent (main memory) storage yet
        storage = std::make_unique<BundleStorageMemory>();
        //  if (frameworkProperties[FWProps::READ_ONLY_PROP] == true)
        //  {
        //    dataStorage.clear();
        //  }
        //  else
        //  {
        //    dataStorage = GetFileStorage(this, "data");
        //  }
        try
        {
            dataStorage = GetPersistentStoragePath(this, "data", /*create=*/false);
        }
        catch (std::exception const& e)
        {
            DIAG_LOG(*sink) << "Ignored runtime exception with message'" << e.what()
                            << "' from the GetPersistentStoragePath function.\n";
        }

        auto bundleValidationFunc = frameworkProperties.find(Constants::FRAMEWORK_BUNDLE_VALIDATION_FUNC);
        if (bundleValidationFunc != frameworkProperties.end())
        {
            validationFunc
                = any_cast<std::function<bool(cppmicroservices::Bundle const&)>>(bundleValidationFunc->second);
        }

        systemBundle->InitSystemBundle();
        US_SET_CTX_FUNC(system_bundle)(systemBundle->bundleContext.Load().get());

        bundleRegistry.Init();

        serviceHooks.Open();

        bundleRegistry.Load();

        logger->Open();

        std::string execPath;
        try
        {
            execPath = util::GetExecutablePath();
        }
        catch (std::exception const& e)
        {
            DIAG_LOG(*sink) << e.what();
            // Let the exception propagate all the way up to the
            // call site of Framework::Init().
            throw;
        }

        if (IsBundleFile(execPath) && bundleRegistry.GetBundles(execPath).empty())
        {
            // Auto-install all embedded bundles inside the executable.
            // Same here: If an embedded bundle cannot be installed,
            // an exception is thrown and we will let it propagate all
            // the way up to the call site of Framework::Init().
            bundleRegistry.Install(execPath, systemBundle.get());
        }

        DIAG_LOG(*sink) << "inited\nInstalled bundles: ";
        for (auto b : bundleRegistry.GetBundles())
        {
            DIAG_LOG(*sink) << " #" << b->id << " " << b->symbolicName << ":" << b->version
                            << " location:" << b->location;
        }

#ifdef US_PLATFORM_POSIX
        try
        {
            libraryLoadOptions = any_cast<int>(frameworkProperties[Constants::LIBRARY_LOAD_OPTIONS]);
        }
        catch (...)
        {
            DIAG_LOG(*sink) << "Unable to read default library load options from config.";
            libraryLoadOptions = RTLD_LAZY | RTLD_LOCAL;

            // override non-integer framework property value to reflect default library load option for POSIX
            frameworkProperties[Constants::LIBRARY_LOAD_OPTIONS] = libraryLoadOptions;
        }
        DIAG_LOG(*sink) << "Library Load Options = " << libraryLoadOptions;
#endif
    }

    void
    CoreBundleContext::Uninit0()
    {
        DIAG_LOG(*sink) << "uninit";
        logger->Close();
        serviceHooks.Close();
        systemBundle->UninitSystemBundle();
    }

    void
    CoreBundleContext::Uninit1()
    {
        bundleRegistry.Clear();
        services.Clear();
        listeners.Clear();
        resolver.Clear();

        storage->Close();
    }

    std::string
    CoreBundleContext::GetDataStorage(long id) const
    {
        if (!dataStorage.empty())
        {
            return dataStorage + util::DIR_SEP + util::ToString(id);
        }
        return std::string();
    }

    WriteLock
    CoreBundleContext::SetFrameworkStopped()
    {
        WriteLock lock(stoppedLock);
        stopped = true;
        return lock;
    }

    WriteLock
    CoreBundleContext::SetFrameworkStarted()
    {
        WriteLock lock(stoppedLock);
        stopped = false;
        return lock;
    }

    std::unique_ptr<BlockFrameworkShutdown>
    CoreBundleContext::GetFrameworkStopped() const
    {
        ReadLock lock(stoppedLock);
        return std::make_unique<BlockFrameworkShutdown>(stopped, std::move(lock));
    }
} // namespace cppmicroservices
