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

#include "ManifestCacheService.h"

#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/GlobalConfig.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleEvent.h"
#include <rapidjson/document.h>

#include <iostream>
#include <chrono>
#include <functional>
#include <sys/stat.h>

namespace {

using namespace std::chrono;
constexpr nanoseconds timespecToDuration(timespec ts)
{
  auto duration = seconds{ts.tv_sec} + nanoseconds{ts.tv_nsec};
    
  return duration_cast<nanoseconds>(duration);
}

constexpr time_point<system_clock, nanoseconds>
timespecToTimePoint(timespec ts)
{
  return time_point<system_clock, nanoseconds> {
    duration_cast<system_clock::duration>(timespecToDuration(ts))
      };
}

}


namespace cppmicroservices {

namespace json {

using AnyOrderedMap = std::map<std::string, cppmicroservices::Any>;
using AnyMap = cppmicroservices::AnyMap;
using AnyVector = std::vector<cppmicroservices::Any>;

US_Framework_EXPORT void ParseJsonObject(const rapidjson::Value& jsonObject, AnyMap& anyMap);
US_Framework_EXPORT void ParseJsonObject(const rapidjson::Value& jsonObject, AnyOrderedMap& anyMap);
US_Framework_EXPORT void ParseJsonArray(const rapidjson::Value& jsonArray,
                                        AnyVector& anyVector,
                                        bool ci);

}

class ManifestCacheServiceImpl
  : public ManifestCacheService 
{
public:
  ManifestCacheServiceImpl()
    : ManifestCacheService()
  {}
  ~ManifestCacheServiceImpl() override {}

  void UpdateCache(const cppmicroservices::BundleEvent& event, BundleContext context) override
  {
    using namespace cppmicroservices;
    // We're only interested in processing BUNDLE_INSTALLED events
    if (BundleEvent::BUNDLE_INSTALLED != event.GetType())
      return;

    // And only interested in processing installed bundles if the bundle is a cache
    auto const& cacheBundle   = event.GetBundle();
    auto const& manifestCache = cacheBundle.GetHeaders();
    auto isManifestCache = any_cast<bool>(manifestCache.at("is_manifest_cache"));
    if (false == isManifestCache)
      return;

    // Loop through the information for the bundles. Skip any entries that are out of
    // date or are for bundles that don't exist on disk
    auto const& bundles = ref_any_cast<cppmicroservices::json::AnyVector>(manifestCache.at("bundles"));
    for (auto const& bundleInfoAny : bundles)
    {
      using namespace std::chrono;
      auto const& bundleInfo = ref_any_cast<AnyMap>(bundleInfoAny);
        
      auto location = bundleInfo.at("file_path").ToString();
      struct stat locationStat;
      if (stat(location.c_str(), &locationStat) == 0)
        // if the bundle exists, then continue with install, otherwise, skip it.
      {
        auto lastModified = system_clock::from_time_t(any_cast<int>(manifestCache.at("created")));
        auto locationTimePoint = timespecToTimePoint(locationStat.st_mtimespec);

        if (locationTimePoint < lastModified)
          // If the cache is more recent than the bundle, install the bundle manifest from the cache.
          // otherwise, ignore this cache entry
        {
          auto const& manifestAny = bundleInfo.at("manifest_contents");
          auto const& manifest = ref_any_cast<cppmicroservices::AnyMap>(manifestAny);
          
          // Finally, add the manifest for this bundle to the registry
          context.InstallBundles(location, manifest);
        }
      }
    }
  }
};

class ManifestCacheServiceActivator
  : public BundleActivator
{
public:
  ManifestCacheServiceActivator() = default;
  ~ManifestCacheServiceActivator() override = default;

  void Start(BundleContext context) override
  {
    cacheService = std::make_shared<ManifestCacheServiceImpl>();
    serviceReg = context.RegisterService<ManifestCacheService>(cacheService);
    context.AddBundleListener([this, context](const BundleEvent& event) { cacheService->UpdateCache(event, context); });
  }

  void Stop(BundleContext) override
  {
    serviceReg.Unregister();
  }

private:
  std::shared_ptr<ManifestCacheServiceImpl> cacheService;
  ServiceRegistration<ManifestCacheService> serviceReg;
};

}

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(cppmicroservices::ManifestCacheServiceActivator)
