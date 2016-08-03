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


#include <usGlobalConfig.h>

US_MSVC_DISABLE_WARNING(4355)

#include "usCoreBundleContext_p.h"
#include "usBundleThread_p.h"

#include "usFramework.h"
#include "usFrameworkPrivate.h"
#include "usBundleStorageMemory_p.h"
#include "usBundleInitialization.h"
#include "usConstants.h"
#include "usBundleUtils_p.h"
#include "usUtils_p.h" // us::ToString()

#include <iomanip>

US_INITIALIZE_BUNDLE

namespace us {

std::atomic<int> CoreBundleContext::globalId{0};

std::map<std::string, Any> InitProperties(std::map<std::string, Any> configuration)
{
  // Framework internal diagnostic logging is off by default
  configuration.insert(std::pair<std::string, Any>(Constants::FRAMEWORK_LOG, false));

  // Framework::PROP_THREADING_SUPPORT is a read-only property whose value is based off of a compile-time switch.
  // Run-time modification of the property should be ignored as it is irrelevant.
  configuration.erase(Constants::FRAMEWORK_THREADING_SUPPORT);
#ifdef US_ENABLE_THREADING_SUPPORT
  configuration.insert(std::pair<std::string, Any>(Constants::FRAMEWORK_THREADING_SUPPORT, std::string("multi")));
#else
  configuration.insert(std::pair<std::string, Any>(Constants::FRAMEWORK_THREADING_SUPPORT, std::string("single")));
#endif

  return configuration;
}

CoreBundleContext::CoreBundleContext(const std::map<std::string, Any>& props, std::ostream* logger)
  : id(globalId++)
  , frameworkProperties(InitProperties(props))
  , listeners(this)
  , services(this)
  , serviceHooks(this)
  , bundleHooks(this)
  , bundleRegistry(this)
  , firstInit(true)
  , initCount(0)
{
  bool enableDiagLog = any_cast<bool>(frameworkProperties.at(Constants::FRAMEWORK_LOG));
  std::ostream* diagnosticLogger = (logger) ? logger : &std::clog;
  sink = std::make_shared<LogSink>(diagnosticLogger, enableDiagLog);
  systemBundle = std::shared_ptr<FrameworkPrivate>(new FrameworkPrivate(this));
  DIAG_LOG(*sink) << "created";
}

CoreBundleContext::~CoreBundleContext()
{
  std::shared_ptr<CoreBundleContext> dummy(this, [](CoreBundleContext*){});
  systemBundle->Shutdown(false);
  systemBundle->WaitForStop(std::chrono::milliseconds(0));
}

void CoreBundleContext::Init()
{
  DIAG_LOG(*sink) << "initializing";
  initCount++;

  auto storageCleanProp = frameworkProperties.find(Constants::FRAMEWORK_STORAGE_CLEAN);
  if (firstInit && storageCleanProp != frameworkProperties.end() &&
      storageCleanProp->second == Constants::FRAMEWORK_STORAGE_CLEAN_ONFIRSTINIT)
  {
    // DeleteFWDir();
    firstInit = false;
  }

  // We use a "pseudo" random UUID.
  const std::string sid_base = "04f4f884-31bb-45c0-b176-";
  std::stringstream ss;
  ss << sid_base << std::setfill('0') << std::setw(8) << std::hex << static_cast<int32_t>(id * 65536 + initCount);

  frameworkProperties[Constants::FRAMEWORK_UUID] = ss.str();

  // $TODO we only support non-persistent (main memory) storage yet
  storage.reset(new BundleStorageMemory());
//  if (frameworkProperties[FWProps::READ_ONLY_PROP] == true)
//  {
//    dataStorage.clear();
//  }
//  else
//  {
//    dataStorage = GetFileStorage(this, "data");
//  }
  dataStorage = GetFileStorage(this, "data");

  systemBundle->InitSystemBundle();
  _us_set_bundle_context_instance_system_bundle(systemBundle->bundleContext.Load().get());

  bundleRegistry.Init();

  serviceHooks.Open();
  //resolverHooks.Open();
  
  // auto-install all embedded bundles inside the executable
  auto execPath = BundleUtils::GetExecutablePath();
  if (IsBundleFile(execPath))
  {
    bundleRegistry.Install(execPath, systemBundle.get(), true);
  }

  bundleRegistry.Load();

  DIAG_LOG(*sink) << "inited\nInstalled bundles: ";
  for (auto b : bundleRegistry.GetBundles())
  {
    DIAG_LOG(*sink) << " #" << b->id << " " << b->symbolicName << ":"
                    << b->version << " location:" << b->location;
  }
}

void CoreBundleContext::Uninit0()
{
  DIAG_LOG(*sink) << "uninit";
  serviceHooks.Close();
  systemBundle->UninitSystemBundle();
}

void CoreBundleContext::Uninit1()
{
  bundleRegistry.Clear();
  services.Clear();
  listeners.Clear();
  resolver.Clear();

  {
    auto l = bundleThreads.Lock(); US_UNUSED(l);
    while (!bundleThreads.value.empty())
    {
      bundleThreads.value.front()->Quit();
      bundleThreads.value.pop_front();
    }
    while (!bundleThreads.zombies.empty())
    {
      bundleThreads.zombies.front()->Join();
      bundleThreads.zombies.pop_front();
    }
  }

  dataStorage.clear();
  storage->Close();
}

std::string CoreBundleContext::GetDataStorage(long id) const
{
  if (!dataStorage.empty())
  {
    return dataStorage + DIR_SEP + us::ToString(id);
  }
  return std::string();
}

}
