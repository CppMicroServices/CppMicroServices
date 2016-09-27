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

#include "BundleStorageMemory.h"
#include "BundleThread.h"
#include "BundleUtils.h"
#include "FrameworkPrivate.h"
#include "Utils.h" // cppmicroservices::ToString()

#include <iomanip>

CPPMICROSERVICES_INITIALIZE_BUNDLE

namespace cppmicroservices {

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
  sink = std::make_shared<detail::LogSink>(diagnosticLogger, enableDiagLog);
  systemBundle = std::shared_ptr<FrameworkPrivate>(new FrameworkPrivate(this));
  DIAG_LOG(*sink) << "created\n";
}

CoreBundleContext::~CoreBundleContext()
{
}

std::shared_ptr<CoreBundleContext> CoreBundleContext::shared_from_this() const
{
  return self.Lock(), self.v.lock();
}

void CoreBundleContext::SetThis(const std::shared_ptr<CoreBundleContext>& self)
{
  this->self.Lock(), this->self.v = self;
}

void CoreBundleContext::Init()
{
  DIAG_LOG(*sink) << "initializing\n";
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

  bundleRegistry.Load();

  auto const execPath = BundleUtils::GetExecutablePath();
  if (bundleRegistry.GetBundles(execPath).empty() &&
      IsBundleFile(execPath))
  {
  // auto-install all embedded bundles inside the executable
    bundleRegistry.Install(execPath, systemBundle.get(), true);
  }

  DIAG_LOG(*sink) << "inited\nInstalled bundles: \n";
  for (auto b : bundleRegistry.GetBundles())
  {
    DIAG_LOG(*sink) << " #" << b->id << " " << b->symbolicName << ":"
                    << b->version << " location:" << b->location << "\n";
  }
}

void CoreBundleContext::Uninit0()
{
  DIAG_LOG(*sink) << "uninit\n";
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
    return dataStorage + DIR_SEP + cppmicroservices::ToString(id);
  }
  return std::string();
}

}
