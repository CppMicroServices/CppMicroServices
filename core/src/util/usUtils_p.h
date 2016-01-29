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


#ifndef USUTILS_H
#define USUTILS_H

#include <usCoreConfig.h>

#include <string>
#include <vector>
#include <memory>

//-------------------------------------------------------------------
// Bundle name and location parsing
//-------------------------------------------------------------------

namespace us {

std::string GetBundleNameFromLocation(const std::string& location);

std::string GetBundleLocation(const std::string& location);
void ExtractBundleNameAndLocation(const std::string& locationName, std::string& oLocation, std::string& oName);
bool IsSharedLibrary(const std::string& location);

}

//-------------------------------------------------------------------
// Bundle auto-loading
//-------------------------------------------------------------------

namespace us {

struct BundleInfo;
class CoreBundleContext;
std::vector<std::string> GetBundleNamesFromLibrary(const std::string& libPath);
std::vector<std::string> AutoInstallBundles(const BundleInfo& bundleInfo, CoreBundleContext* coreCtx);
std::vector<std::string> AutoInstallBundlesFromPath(const std::string& absoluteBasePath, 
	const std::string& subDir = std::string());
}

//-------------------------------------------------------------------
// Generic utility functions
//-------------------------------------------------------------------

namespace us {

// A convenient way to construct a shared_ptr holding an array
template<typename T> std::shared_ptr<T> make_shared_array(std::size_t size)
{
  return std::shared_ptr<T>(new T[size], std::default_delete<T[]>());
}

// Platform agnostic way to get the current working directory.
// Supports Linux, Mac, and Windows.
std::string GetCurrentWorkingDirectory();

}

//-------------------------------------------------------------------
// Error handling
//-------------------------------------------------------------------

namespace us {

US_Core_EXPORT std::string GetLastErrorStr();

}

#endif // USUTILS_H
