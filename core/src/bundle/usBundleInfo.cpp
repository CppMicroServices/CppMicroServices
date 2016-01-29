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


#include "usBundleInfo.h"
#include "usUtils_p.h"

#include <stdlib.h>
namespace us {

  BundleInfo::BundleInfo(const std::string& location, const std::string& name)
  : name(name)
  , id(0)
{
#if _WIN32
	this->location = location;
#else
  // resolve any symlinks. 
  char filePath[PATH_MAX];
  char* resultPath = ::realpath(location.c_str(), filePath);
  if (resultPath) {
    this->location = std::string(filePath);
  }
#endif
  if (name.empty())
  {
    embeddedBundles = GetBundleNamesFromLibrary(location);
    if (embeddedBundles.size())
    {
      this->name = embeddedBundles.at(0);
      // remove the top most element which is the current bundle name
      embeddedBundles.erase(embeddedBundles.begin());
    }
  }
  else if(name == US_CORE_FRAMEWORK_NAME)
  {
    // This is a special case where we know the location contains the framework
    // check for other bundles embedded in the same binary
    embeddedBundles = GetBundleNamesFromLibrary(location);
    std::vector<std::string>::iterator it = embeddedBundles.begin();
    while (it != embeddedBundles.end())
    {
      if (*it == US_CORE_FRAMEWORK_NAME)
      {
        it = embeddedBundles.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }
}

}
