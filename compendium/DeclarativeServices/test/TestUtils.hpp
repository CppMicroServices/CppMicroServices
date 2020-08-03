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

#ifndef TestUtils_hpp
#define TestUtils_hpp

#include <cppmicroservices/ServiceReference.h>
#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>

#include <random>
#include <string>

namespace test {

template<typename Task, typename Predicate>
bool RepeatTaskUntilOrTimeout(Task&& t, Predicate&& p)
{
  using namespace std::chrono;
  auto startTime = system_clock::now();
  do {
    t();
    duration<double> duration = system_clock::now() - startTime;
    if (duration > milliseconds(30000)) {
      return false;
    }
  } while (!p());
  return true;
}

/**
 * Convenience Method to install but not start a bundle given the bundle's symbolic name.
 */
void InstallLib(::cppmicroservices::BundleContext frameworkCtx, const std::string& libName);

/**
 * Convenience Method to install and start a bundle given the bundle's symbolic name.
 */
cppmicroservices::Bundle InstallAndStartBundle(::cppmicroservices::BundleContext frameworkCtx, const std::string& libName);

/**
 * Convenience Method to install and start DS.
 */
void InstallAndStartDS(::cppmicroservices::BundleContext frameworkCtx);

/**
 * Convenience Method to extract service-id from a service reference
 */
long GetServiceId(const ::cppmicroservices::ServiceReferenceBase& sRef);

/**
 * Returns the file path of declarative services runtime plugin
 */
std::string GetDSRuntimePluginFilePath();

/**
 * Returns the path to the test bundles folder
 */
std::string GetTestPluginsPath();

} // namespaces

#endif /* TestUtils_hpp */
