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

#include "TestUtils.hpp"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/util/FileSystem.h"
#include <iostream>
#include "DSTestingConfig.h"

namespace {

std::string PathToLib(const std::string& libName)
{
  return (cppmicroservices::testing::LIB_PATH
          + cppmicroservices::util::DIR_SEP
          + US_LIB_PREFIX
          + libName
          + US_LIB_POSTFIX
          + US_LIB_EXT);
}

}

namespace test {

void InstallLib(
#if defined(US_BUILD_SHARED_LIBS)
  ::cppmicroservices::BundleContext frameworkCtx
  , const std::string& libName
#else
  ::cppmicroservices::BundleContext
  , const std::string&
#endif
  )
{
#if defined(US_BUILD_SHARED_LIBS)
  frameworkCtx.InstallBundles(PathToLib(libName));
#endif
}

cppmicroservices::Bundle InstallAndStartBundle(::cppmicroservices::BundleContext frameworkCtx, const std::string& libName)
{
  std::vector<cppmicroservices::Bundle> bundles;

#if defined(US_BUILD_SHARED_LIBS)
  bundles = frameworkCtx.InstallBundles(cppmicroservices::testing::LIB_PATH
        + cppmicroservices::util::DIR_SEP
        + US_LIB_PREFIX
        + libName
        + US_LIB_POSTFIX
        + US_LIB_EXT);
#else
  bundles = frameworkCtx.GetBundles();
#endif

  for (auto b : bundles) {
    if (b.GetSymbolicName() == libName) {
        b.Start();
        return b;
    }
  }
  return {};
}

long GetServiceId(const ::cppmicroservices::ServiceReferenceBase& sRef)
{
  long serviceId = 0;
  try 
  {
    if(sRef)
    {
      serviceId = cppmicroservices::any_cast<long int>(sRef.GetProperty(::cppmicroservices::Constants::SERVICE_ID));
    }
  }
  catch (const std::exception& e)
  {
    std::cout << "Exception: " << e.what() << std::endl;
    throw;
  }
  return serviceId;
}

std::string GetDSRuntimePluginFilePath()
{
  std::string libName { "DeclarativeServices" };
#if defined(US_PLATFORM_WINDOWS)
  libName += US_DeclarativeServices_VERSION_MAJOR;
#endif
  return PathToLib(libName);
}

std::string GetTestPluginsPath()
{
  return (cppmicroservices::testing::LIB_PATH
          + cppmicroservices::util::DIR_SEP);
}

} // namespaces
